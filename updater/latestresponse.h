#pragma once

#include <QString>
#include <QVersionNumber>
#include <QByteArray>

class LatestResponse
{
public:
    bool load(const QByteArray &json);

    bool success() const;
    bool updateAvailable() const;
    bool mandatory() const;

    QVersionNumber latestVersion() const;

    QString manifestUrl() const;

private:
    bool m_success = false;
    bool m_updateAvailable = false;
    bool m_mandatory = false;

    QVersionNumber m_latestVersion;

    QString m_manifestUrl;
};
