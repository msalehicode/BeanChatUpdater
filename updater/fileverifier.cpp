#include "fileverifier.h"

#include <QFile>
#include <QCryptographicHash>

QString FileVerifier::sha256(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return {};

    QByteArray hash =
        QCryptographicHash::hash(
            file.readAll(),
            QCryptographicHash::Sha256);

    return hash.toHex();
}
