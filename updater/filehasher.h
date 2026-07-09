#pragma once

#include <QString>

class FileHasher
{
public:
    static QString sha256(const QString &filePath);
};
