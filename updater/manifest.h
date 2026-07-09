#pragma once

#include <QVersionNumber>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QVector>
#include <QDate>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "manifestfile.h"

class Manifest
{
public:

    bool load(const QByteArray &json);

    QVersionNumber version() const;

    QString baseUrl() const;

    const QVector<ManifestFile>& files() const;


    QString app() const;
    QString platform() const;
    QVersionNumber minimumUpdaterVersion() const;
    QDate releaseDate() const;
    QStringList releaseNotes() const;
private:

    QString m_app;

    QString m_platform;

    QVersionNumber m_version;

    QVersionNumber m_minimumUpdaterVersion;

    QString m_baseUrl;

    QDate m_releaseDate;

    QStringList m_releaseNotes;

    QVector<ManifestFile> m_files;
};
