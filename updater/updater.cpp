#include "updater.h"
#include "filehasher.h"
#include "fileverifier.h"

#include <QStandardPaths>
#include <QDir>
#include <QProcess>

Updater::Updater(QObject *parent)
    : QObject(parent)
{
    connect(
        &m_downloader,
        &Downloader::finished,
        this,
        [this](Downloader::DownloadType type,
               const QByteArray &data)
        {
            appendLog("Download finished.");

            switch(type)
            {
            case Downloader::DownloadType::LatestResponse:
            {
                if(!m_latestResponse.load(data))
                {
                    setState(State::Error);
                    setTitle("Error");
                    setDescription("failed to get latest");
                    appendLog("error: can't load latest.");
                    emit showCloseButton();
                    return;
                }

                if(!m_latestResponse.success())
                {
                    setState(State::Error);
                    setTitle("Error");
                    setDescription("failed to get latest");
                    appendLog("error: latest response wasn't successful.");
                    emit showCloseButton();
                    emit logChanged();
                    return;
                }

                if(!m_latestResponse.updateAvailable())
                {
                    setTitle("Already up to date");
                    setDescription("your app is already up to date");
                    appendLog("Already up to date.");
                    setState(State::Finished);
                    emit nothingToDo();
                    emit logChanged();
                    launchApplication();
                    return;
                }

                setState(State::DownloadingManifest);
                setTitle("Downloading manifest");
                setDescription("Reading update information");
                appendLog("Downloading manifest");

                m_downloader.download(
                    Downloader::DownloadType::Manifest,
                    QUrl(m_latestResponse.manifestUrl()));

                break;
            }

            case Downloader::DownloadType::Manifest:
            {
                if(!m_manifest.load(data))
                {
                    setState(State::Error);
                    setTitle("Error");
                    setDescription("failed to load manifest");
                    appendLog("can't load manifest.");
                    emit showCloseButton();
                    emit logChanged();
                    return;
                }

                setState(State::DownloadingManifest);
                setTitle("Downloading manifest");
                setDescription("Reading update information");
                appendLog("Manifest loaded.");


                //add version directory to temp
                m_tempDirectory = QDir(m_tempDirectory).filePath(m_manifest.version().toString());
                if(!QDir().mkpath(m_tempDirectory))
                {
                    appendLog("failed to create temporary directory.");
                }
                else
                {
                    appendLog("temporary directory created.");
                }

                compareLocalFiles();

                break;
            }

            case Downloader::DownloadType::File:
            {
                fileDownloaded(data);
                break;
            }

            default:
                break;
            }
        });

    connect(
        &m_downloader,
        &Downloader::errorOccurred,
        this,
        [this](const QString &error)
        {
            setState(State::Error);
            setTitle("Network Error");
            setDescription(error);
            appendLog("network error: "+ error);
            emit showCloseButton();
            emit logChanged();
        });
}


void Updater::start()
{
    QString temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_tempDirectory = QDir(temp).filePath("BeanChatUpdater");


    setState(State::ParsingArguments);
    setTitle("Starting...");
    setDescription("Reading launch arguments");
    appendLog("received launch arguments are:");

    QString receivedArgs;
    if(!m_arguments.parse(receivedArgs))
    {
        setState(State::Error);
        setTitle("Invalid arguments");
        setDescription("missing arguments, please try again");
        appendLog("failed: missing arguments (version is a must)");
        emit showCloseButton();
        emit logChanged();
        return;
    }
    appendLog(receivedArgs);


    //version didnt pass by argument try to find it from QSettings
    if(m_arguments.currentVersion().isNull())
    {
        appendLog("current version isn't provided, let's try find it");

        //try to read version from settings
        QString foundVersion="";
        if(m_settings.contains("App/Version"))
        {
            foundVersion=m_settings.value("App/Version","").toString();
            if(!foundVersion.isEmpty())
            {
                m_arguments.setCurrentVersion(foundVersion);
                appendLog("found version: "+foundVersion);
            }
        }
        else
        {
            setState(State::Error);
            setTitle("Invalid version");
            setDescription("if you've never run BeanChat.exe run it then run updater manually  OR  *update by  BeanChat -> settings -> update*");
            appendLog("failed to read App/Version, you must first launch app OR let BeanChat run updater (go to settings -> update)");
            emit showCloseButton();
            emit logChanged();
            return;
        }
    }

    appendLog("current version is: "+m_arguments.currentVersion().toString());

    m_installDirectory = QCoreApplication::applicationDirPath();
    if (!m_arguments.installDirectory().isEmpty())
    {
        m_installDirectory = m_arguments.installDirectory();
        appendLog("install directory not speicified, assuming it's: " + m_installDirectory);
    }


    if (m_arguments.repair())
    {
        startRepair();
    }
    else
    {
        setState(State::DownloadingLatestResponse);
        setTitle("Checking for updates");
        QUrl targetUrl(m_latestPath);
        setDescription("Contacting to "+targetUrl.host());
        appendLog("Connecting to "+m_latestPath);

        m_downloader.download(
            Downloader::DownloadType::LatestResponse,
            QUrl(m_latestPath + "?platform=" + m_platform
                 + "&version=" + m_arguments.currentVersion().toString()));
    }
}

void Updater::cancelUpdate()
{
    setState(State::Canceled);
    setTitle("Update Canceled");
    setDescription("");
    appendLog("update canceled.");
    emit updatedCanceled();
    emit logChanged();
}

void Updater::confirmUpdate()
{
    appendLog("update confirmed, let's proceed...");
    m_currentDownload = 0;
    downloadNextFile();
}


void Updater::compareLocalFiles()
{
    setState(State::ComparingFiles);
    setTitle("Comparing files");
    setDescription("Looking for changed files");
    appendLog("===== Comparing Files =====");

    for(const ManifestFile &file : m_manifest.files())
    {
        QString localFile = QDir(m_installDirectory).filePath(file.path);

        QFileInfo info(localFile);

        if(!info.exists())
        {
            appendLog("[MISSING] "+file.path);
            m_downloadQueue.push_back(file);

            continue;
        }

        QString hash = FileHasher::sha256(localFile);

        if(hash.compare(file.sha256, Qt::CaseInsensitive) == 0)
        {
            // appendLog("[OK] " + file.path);
            qDebug() << "ok " << file.path;
        }
        else
        {
            appendLog("[DIFFERENT] " +file.path + " local: " + hash + " expected: " + file.sha256);

            m_downloadQueue.push_back(file);
        }
    }

    appendLog("===========================");

    if(m_downloadQueue.isEmpty())
    {
        setState(State::Finished);
        setTitle("Done");
        setDescription("Everything is already up to date.");
        appendLog("Everything is already up to date.");
        emit nothingToDo();
        emit logChanged();
        launchApplication();
        return;
    }

    appendLog("Need to download " + QString::number(m_downloadQueue.size()) + " files.");
    qint64 totalSizeToDownload=0;
    for(const auto &file : m_downloadQueue)
    {
        appendLog(file.path + " " + formatBytes(file.size));
        totalSizeToDownload+=file.size;
    }

    appendLog("===========================");

    setState(State::WaitForConfirmation);
    setTitle("Confirm to update");
    setDescription("please confirm to update or cancel update");
    appendLog("wait for user to confirm update.");
    emit confirmUpdateOrCancel(formatBytes(totalSizeToDownload));
    emit logChanged();
}

void Updater::verifyDownloadedFiles()
{
    appendLog("===== Verifying Files =====");

    QString tempDir = QDir::tempPath() + "/BeanChatUpdater/" + m_manifest.version().toString();

    for (const ManifestFile &file : m_downloadQueue)
    {
        QString path = tempDir + "/" + file.path;

        QString hash = FileVerifier::sha256(path);

        if (hash.compare(file.sha256, Qt::CaseInsensitive) != 0)
        {
            appendLog("[FAILED] " + file.path);
            setState(State::Error);
            return;
        }

        appendLog("[OK] "+file.path);
    }

    appendLog("All files verified.");

    // Next step later:
    setState(State::Installing);
    setTitle("Installing");
    setDescription("Replacing application files");
    appendLog("replacing application files.");
    installFiles();
}

void Updater::installFiles()
{
    appendLog("===== Installing Files =====");

    // QString installDir = m_arguments.installDirectory();

    QString tempDir = QDir::tempPath() + "/BeanChatUpdater/" + m_manifest.version().toString();

    for (const ManifestFile &file : m_downloadQueue)
    {
        QString source = tempDir + "/" + file.path;


        QString destination = m_installDirectory + "/" + file.path;
        appendLog("Installing: " + file.path);

        // create folders if needed
        appendLog("Creating directory");
        QDir().mkpath(QFileInfo(destination).absolutePath());

        // remove old file
        appendLog("Removing old file: ");
        if (QFile::exists(destination))
        {
            if (!QFile::remove(destination))
            {
                appendLog("Couldn't remove"+ destination);
                setState(State::Error);
                return;
            }
        }

        // copy new file
        appendLog("Copying file " + source + " to " + destination);
        if (!QFile::copy(source, destination))
        {
            appendLog("Couldn't copy" + source+ " -> " + destination);
            setState(State::Error);
            return;
        }

        appendLog("[INSTALLED] " +file.path);
    }
    appendLog("Installation completed.");


    //update qsettings app version
    m_settings.setValue("App/Version",m_latestResponse.latestVersion().toString());


    setState(State::Finished);
    setTitle("Updated Successfully");
    setDescription("");
    emit updatedSuccessfully();
    emit logChanged();
    launchApplication();
}

void Updater::launchApplication()
{
    QString exe = QDir(m_installDirectory).filePath("BeanChat.exe");

    // setState(State::Launching);
    // setTitle("Launching..");
    // setDescription("launcing BeanChat..");
    appendLog("trying to launch App BeanChat. launch target: "+exe);
    if (!QProcess::startDetached(exe))
    {
        // setState(State::Error);
        // setTitle("Failed to launch..");
        // setDescription("launcing BeanChat failed, please launch it manually.");
        appendLog("Failed to launch "+exe);
        // emit showCloseButton();
        emit logChanged();
        return;
    }

    // setState(State::Finished);
    // setTitle("Launched.");
    // setDescription("App BeanChat launched.");
    appendLog("App BeanChat launched.");
    emit logChanged();
    QCoreApplication::quit();
}

void Updater::closeApplication()
{
    appendLog("closing");
    QCoreApplication::quit();
}

QString Updater::log() const
{
    return m_log;
}

void Updater::appendLog(const QString &msg)
{
    m_log += "\n"+ msg;
    // qDebug() << msg;
    emit logChanged();
}

void Updater::startRepair()
{
    appendLog("Repair mode is ON");

    QString manifest = QString(m_manifestPath + m_platform + "/%1.json")
                           .arg(m_arguments.currentVersion().toString());

    m_downloader.download(
        Downloader::DownloadType::Manifest,
        QUrl(manifest));
    setState(State::DownloadingManifest);
    setTitle("Downloading repair manifest");
    setDescription("downloading manifest to repair version " + m_arguments.currentVersion().toString());
    appendLog("downloading manifest to repair version " + m_arguments.currentVersion().toString()+ " from " + m_manifestPath);
}

bool Updater::busy() const
{
    return m_busy;
}

void Updater::setBusy(bool newBusy)
{
    if (m_busy == newBusy)
        return;
    m_busy = newBusy;
    emit busyChanged();
}

float Updater::progress() const
{
    return m_progress;
}

void Updater::setProgress(float newProgress)
{
    if (qFuzzyCompare(m_progress, newProgress))
        return;
    m_progress = newProgress;
    emit progressChanged();
}

QString Updater::description() const
{
    return m_description;
}

void Updater::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
    emit descriptionChanged();
}

QString Updater::title() const
{
    return m_title;
}

void Updater::setTitle(const QString &newTitle)
{
    if (m_title == newTitle)
        return;
    m_title = newTitle;
    emit titleChanged();
}


void Updater::downloadNextFile()
{
    if(m_currentDownload >= m_downloadQueue.size())
    {
        appendLog("===========================");
        appendLog("All downloads finished.");
        appendLog("===========================");


        if (!m_downloadQueue.isEmpty())
        {
            setState(State::VerifyingFiles);
            setTitle("Verifying");
            setDescription("Checking downloaded files");
            verifyDownloadedFiles();
        }

        return;
    }

    appendLog("===========================");
    const ManifestFile &file = m_downloadQueue[m_currentDownload];

    QUrl url(m_manifest.baseUrl() + file.path);


    setState(State::DownloadingFiles);
    setTitle("Downloading update");
    setProgress(float(m_currentDownload) / m_downloadQueue.size());
    setDescription("Downloading "+QString::number(m_currentDownload+1)+ " of "+ QString::number(m_downloadQueue.size()));
    appendLog("Downloading "+file.path + " - URL=" + url.toString());

    m_downloader.download(
        Downloader::DownloadType::File,
        url);
}

QString Updater::formatBytes(qint64 bytes)
{
    static const char* units[] = { "B", "KB", "MB", "GB", "TB" };

    double size = static_cast<double>(bytes);
    int unit = 0;

    while (size >= 1024.0 && unit < 4)
    {
        size /= 1024.0;
        ++unit;
    }

    return QString("%1 %2")
        .arg(size, 0, 'f', unit == 0 ? 0 : 2)
        .arg(units[unit]);
}

void Updater::fileDownloaded(const QByteArray &data)
{
    const ManifestFile &file = m_downloadQueue[m_currentDownload];

    appendLog("Downloaded "+ file.path + " size: " + formatBytes(data.size()));

    QString destination = QDir(m_tempDirectory).filePath(file.path);

    QFileInfo info(destination);
    QDir().mkpath(info.absolutePath());

    QFile output(destination);
    if(!output.open(QIODevice::WriteOnly))
    {
        appendLog("Cannot write to " + destination);
        setState(State::Error);
        return;
    }
    output.write(data);
    output.close();
    appendLog("File saved to " + destination);
    m_currentDownload++;

    downloadNextFile();
}

Updater::State Updater::state() const
{
    return m_state;
}


void Updater::setState(State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged();
}
