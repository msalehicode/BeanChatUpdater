#pragma once

#include <QString>

class FileVerifier
{
public:
    static QString sha256(const QString &filePath);
};
