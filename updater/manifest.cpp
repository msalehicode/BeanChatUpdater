#include "manifest.h"


QVersionNumber Manifest::version() const
{
    return m_version;
}

QString Manifest::baseUrl() const
{
    return m_baseUrl;
}

const QVector<ManifestFile>& Manifest::files() const
{
    return m_files;
}


QString Manifest::app() const
{
    return m_app;
}

QString Manifest::platform() const
{
    return m_platform;
}

QVersionNumber Manifest::minimumUpdaterVersion() const
{
    return m_minimumUpdaterVersion;
}

QDate Manifest::releaseDate() const
{
    return m_releaseDate;
}

QStringList Manifest::releaseNotes() const
{
    return m_releaseNotes;
}

bool Manifest::load(const QByteArray &json)
{
    m_app.clear();
    m_platform.clear();
    m_version = {};
    m_minimumUpdaterVersion = {};
    m_baseUrl.clear();
    m_releaseDate = QDate();
    m_releaseNotes.clear();
    m_files.clear();


    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(json, &error);

    if (document.isNull())
    {
        qWarning() << error.errorString();
        return false;
    }

    if (!document.isObject())
    {
        qWarning() << "Manifest root is not an object.";
        return false;
    }

    QJsonObject root = document.object();

    m_app =
        root["app"].toString();

    m_platform =
        root["platform"].toString();

    m_version =
        QVersionNumber::fromString(
            root["version"].toString());

    m_minimumUpdaterVersion =
        QVersionNumber::fromString(
            root["minimumUpdaterVersion"].toString());

    m_baseUrl =
        root["baseUrl"].toString();

    m_releaseDate =
        QDate::fromString(
            root["releaseDate"].toString(),
            Qt::ISODate);


    QJsonArray notes =
        root["releaseNotes"].toArray();

    for (const QJsonValue &note : notes)
    {
        m_releaseNotes.push_back(
            note.toString());
    }


    if (m_version.isNull())
    {
        qWarning() << "Invalid version.";
        return false;
    }

    // Base URL
    if (m_baseUrl.isEmpty())
    {
        qWarning() << "Missing baseUrl.";
        return false;
    }

    // Files
    QJsonArray files =
        root["files"].toArray();

    for (const QJsonValue &value : files)
    {
        if (!value.isObject())
            continue;

        QJsonObject object = value.toObject();

        ManifestFile file;

        file.path =
            object["path"].toString();

        file.sha256 =
            object["sha256"].toString();

        file.executable =
            object["executable"].toBool();

        file.size =
            object["size"].toInteger();

        if (file.path.isEmpty())
            continue;

        m_files.push_back(file);
    }

    qDebug() << "========== Manifest ==========";
    qDebug() << "App:" << m_app;
    qDebug() << "Platform:" << m_platform;
    qDebug() << "Version:" << m_version.toString();
    qDebug() << "Min updater:" << m_minimumUpdaterVersion.toString();
    qDebug() << "Base URL:" << m_baseUrl;
    qDebug() << "Release Date:" << m_releaseDate.toString(Qt::ISODate);
    qDebug() << "Files:" << m_files.size();

    for (const QString &note : m_releaseNotes)
    {
        qDebug() << " -" << note;
    }

    qDebug() << "==============================";

    return true;
}
