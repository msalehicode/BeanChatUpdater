#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Downloader : public QObject
{
    Q_OBJECT

public:
    enum class DownloadType
    {
        Unknown = 0,
        LatestResponse,
        Manifest,
        File
    };

    explicit Downloader(QObject *parent = nullptr);

    void download(DownloadType type, const QUrl &url);

signals:

    void finished(DownloadType type, const QByteArray &data);

    void errorOccurred(const QString &message);

    void downloadProgress(qint64 received, qint64 total);

private:

    QNetworkAccessManager m_network;
};
