#pragma once

#include <QString>
#include <QCoreApplication>
#include <QDebug>
#include <QVersionNumber>

class Arguments
{
public:
    bool parse(QString& received);



    bool repair() const;
    QString installDirectory() const;
    QString manifestUrl() const;
    QVersionNumber currentVersion() const;
    void setCurrentVersion(const QString& version);

private:
    bool m_repair = false;

    QString m_installDirectory;
    QString m_manifestUrl;
    QVersionNumber m_currentVersion;
};
