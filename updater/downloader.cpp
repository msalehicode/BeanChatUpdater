#include "downloader.h"

Downloader::Downloader(QObject *parent)
    : QObject(parent)
{

}

void Downloader::download(DownloadType type, const QUrl &url)
{
    qDebug() << "Downloading:" << url;

    QNetworkReply *reply =
        m_network.get(
            QNetworkRequest(url));

    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [this, reply, type]()
        {
            if(reply->error() != QNetworkReply::NoError)
            {
                emit errorOccurred(
                    reply->errorString());

                reply->deleteLater();

                return;
            }

            emit finished(
                type,
                reply->readAll());

            reply->deleteLater();
        });
}
