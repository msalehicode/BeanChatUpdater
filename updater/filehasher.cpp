#include "filehasher.h"

#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

QString FileHasher::sha256(const QString &filePath)
{
    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open:" << filePath;
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while(!file.atEnd())
        hash.addData(file.read(1024 * 1024)); //1MB chunks

    return hash.result().toHex();
}
