#pragma once

#include <QString>

class ManifestFile
{
public:

    QString path;

    QString sha256;

    bool executable = false; //The updater can automatically restore executable permissions on Linux/macOS only for the files that need them.

    qint64 size = 0;
};
