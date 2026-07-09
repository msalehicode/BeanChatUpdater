#include "latestresponse.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

bool LatestResponse::load(const QByteArray &json)
{
    m_success = false;
    m_updateAvailable = false;
    m_mandatory = false;
    m_latestVersion = {};
    m_manifestUrl.clear();

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            json,
            &error);

    if(document.isNull())
    {
        qWarning() << error.errorString();
        return false;
    }

    if(!document.isObject())
    {
        qWarning() << "Latest response is not an object.";
        return false;
    }

    QJsonObject root =
        document.object();

    m_success =
        root["success"].toBool();

    m_updateAvailable =
        root["updateAvailable"].toBool();

    m_mandatory =
        root["mandatory"].toBool();

    m_latestVersion =
        QVersionNumber::fromString(
            root["latestVersion"].toString());

    m_manifestUrl =
        root["manifest"].toString();

    qDebug() << "====== Latest Response ======";
    qDebug() << "Success :" << m_success;
    qDebug() << "Update? :" << m_updateAvailable;
    qDebug() << "Mandatory:" << m_mandatory;
    qDebug() << "Version :" << m_latestVersion.toString();
    qDebug() << "Manifest:" << m_manifestUrl;
    qDebug() << "=============================";

    return true;
}


bool LatestResponse::success() const
{
    return m_success;
}

bool LatestResponse::updateAvailable() const
{
    return m_updateAvailable;
}

bool LatestResponse::mandatory() const
{
    return m_mandatory;
}

QVersionNumber LatestResponse::latestVersion() const
{
    return m_latestVersion;
}

QString LatestResponse::manifestUrl() const
{
    return m_manifestUrl;
}
