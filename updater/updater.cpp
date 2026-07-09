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
                    appendLog("error: latestResponse cant load.");
                    return;
                }

                if(!m_latestResponse.success())
                {
                    setState(State::Error);
                    appendLog("error: latestResponse wasn't successful.");
                    return;
                }

                if(!m_latestResponse.updateAvailable())
                {
                    appendLog("Already up to date.");
                    setTitle("Done");
                    setDescription("Already up to date");
                    setState(State::Launching);
                    launchApplication();
                    return;
                }

                setState(State::DownloadingManifest);
                setTitle("Downloading manifest");
                setDescription("Reading update information");

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
                    return;
                }

                appendLog("Manifest loaded.");
                setState(State::DownloadingManifest);
                setTitle("Downloading manifest");
                setDescription("Reading update information");


                //add version directory to temp
                m_tempDirectory = QDir(m_tempDirectory).filePath(m_manifest.version().toString());
                QDir().mkpath(m_tempDirectory);


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
            appendLog("error: "+ error);

            setState(State::Error);
        });
}


void Updater::start()
{
    QString temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_tempDirectory = QDir(temp).filePath("BeanChatUpdater");


    setState(State::ParsingArguments);
    setTitle("Starting...");
    setDescription("Reading launch options");
    appendLog("reading launch options:");

    QString receivedArgs;
    if(!m_arguments.parse(receivedArgs))
    {
        setState(State::Error);
        return;
    }

    m_installDirectory = m_arguments.installDirectory();

    appendLog(receivedArgs);
    if (m_arguments.repair())
    {
        startRepair();
    }
    else
    {
        setState(State::DownloadingLatestResponse);
        setTitle("Checking for updates");
        setDescription("Contacting BeanChat server");
        appendLog("Connecting BeanChat server");

        m_downloader.download(
            Downloader::DownloadType::LatestResponse,
            QUrl("https://beanchat1.ir/bc/api/latest.php?platform=windows-x64&version=0.40.5"));
    }




}

void Updater::skipUpdate()
{
    setState(State::Launching);
    appendLog("updated skipped.. launching...");
    setTitle("Update Skipped, Launching...");
    setDescription("Update skipped");
    launchApplication();
}

void Updater::confirmUpdate()
{
    appendLog("updated confirmed updating...");
    m_currentDownload = 0;
    downloadNextFile();
}


void Updater::compareLocalFiles()
{
    appendLog("===== Comparing Files =====");

    setState(State::ComparingFiles);
    setTitle("Comparing files");
    setDescription("Looking for changed files");

    for(const ManifestFile &file : m_manifest.files())
    {
        QString localFile =
            QDir(m_installDirectory)
                .filePath(file.path);

        QFileInfo info(localFile);

        if(!info.exists())
        {
            appendLog("[MISSING] "+file.path);

            m_downloadQueue.push_back(file);

            continue;
        }

        QString hash =
            FileHasher::sha256(localFile);

        if(hash.compare(file.sha256, Qt::CaseInsensitive) == 0)
        {
            appendLog("[OK] " + file.path);
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
        appendLog("Everything is already up to date.");
        setState(State::Launching);
        setTitle("Done");
        setDescription("Everything is already up to date.");
        launchApplication();
        return;
    }

    appendLog("Need to download" + QString::number(m_downloadQueue.size()) + "files.");
    for(const auto &file : m_downloadQueue)
    {
        appendLog(file.path + QString::number(file.size));
    }

    appendLog("===========================");

    setState(State::WaitForConfirmation);
    setTitle("Please Confirm");
    setDescription("Cofirm to update or skip");
}

void Updater::verifyDownloadedFiles()
{
    appendLog("===== Verifying Files =====");

    QString tempDir =
        QDir::tempPath()
        + "/BeanChatUpdater/"
        + m_manifest.version().toString();

    for (const ManifestFile &file : m_manifest.files())
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
    installFiles();
}

void Updater::installFiles()
{
    setState(State::Installing);

    appendLog("===== Installing Files =====");

    QString installDir =
        m_arguments.installDirectory();

    QString tempDir =
        QDir::tempPath()
        + "/BeanChatUpdater/"
        + m_manifest.version().toString();

    for (const ManifestFile &file : m_manifest.files())
    {
        QString source =
            tempDir + "/" + file.path;

        QString destination =
            installDir + "/" + file.path;

        // create folders if needed
        QDir().mkpath(
            QFileInfo(destination).absolutePath());

        // remove old file
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
        if (!QFile::copy(source, destination))
        {
            appendLog("Couldn't copy" + source+ "->" + destination);

            setState(State::Error);
            return;
        }

        appendLog("[INSTALLED] " +file.path);
    }

    appendLog("Installation complete.");

    setState(State::Launching);


    //code here..
    appendLog("Launching BeanChat..");
    launchApplication();
}

void Updater::launchApplication()
{
    QString exe = QDir(m_installDirectory).filePath("appBeanChat.exe");

    setState(State::Launching);
    setTitle("Launching..");
    setDescription("launcing BeanChat..");



    if (!QProcess::startDetached(exe))
    {
        appendLog("Failed to launch "+exe);

        // setState(State::Error);
        return;
    }

    appendLog("BeanChat launched.");

    setState(State::Finished);

    QCoreApplication::quit();
}

QString Updater::log() const
{
    return m_log;
}

void Updater::setLog(const QString &newLog)
{
    if (m_log == newLog)
        return;
    m_log = newLog;
    emit logChanged();
}

void Updater::appendLog(const QString &msg)
{
    m_log += "\n"+ msg;
    emit logChanged();
}

void Updater::startRepair()
{
    appendLog("Repair mode");

    QString manifest =
        QString("https://beanchat1.ir/bc/manifests/windows-x64/%1.json")
            .arg(m_arguments.currentVersion().toString());

    m_downloader.download(
        Downloader::DownloadType::Manifest,
        QUrl(manifest));
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

    const ManifestFile &file =
        m_downloadQueue[m_currentDownload];

    QUrl url(
        m_manifest.baseUrl() + file.path);

    appendLog("Downloading "+file.path);

    setState(State::DownloadingFiles);
    setTitle("Downloading update");
    setProgress(float(m_currentDownload) / m_downloadQueue.size());
    setDescription("Downloading "+QString::number(m_currentDownload)+ " of "+ QString::number(m_downloadQueue.size()));

    appendLog("from "+url.toString());

    m_downloader.download(
        Downloader::DownloadType::File,
        url);
}

void Updater::fileDownloaded(const QByteArray &data)
{
    const ManifestFile &file =
        m_downloadQueue[m_currentDownload];

    appendLog("Downloaded "+ file.path + QString::number(data.size()) + " bytes");

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
    appendLog("file has saved. path: " + destination);

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
