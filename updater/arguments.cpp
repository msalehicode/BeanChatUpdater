#include "arguments.h"


bool Arguments::parse(QString &received)
{
    QStringList args = QCoreApplication::arguments();

    for (int i = 1; i < args.size(); ++i)
    {
        const QString &arg = args[i];

        if (arg == "-repair")
        {
            m_repair = true;
        }
        else if (arg == "-installDir" && i + 1 < args.size())
        {
            m_installDirectory = args[++i];
        }
        else if (arg == "-manifest" && i + 1 < args.size())
        {
            m_manifestUrl = args[++i];
        }
        else if (arg == "-version" && i + 1 < args.size())
        {
            m_currentVersion = QVersionNumber::fromString(args[++i]);
        }
        else
        {
            qWarning() << "Unknown argument:" << arg;
        }
    }

    received+= "\n===== Parsed Arguments =====";
    received+= "\nRepair:" + repair();
    received+= "\nInstall Dir:" + installDirectory();
    received+= "\nManifest:" + manifestUrl();
    received+= "\nCurrent Version:" + currentVersion().toString();
    received+= "\n============================";

    return true;
}

bool Arguments::repair() const
{
    return m_repair;
}

QString Arguments::installDirectory() const
{
    return m_installDirectory;
}

QString Arguments::manifestUrl() const
{
    return m_manifestUrl;
}

QVersionNumber Arguments::currentVersion() const
{
    return m_currentVersion;
}

void Arguments::setCurrentVersion(const QString &version)
{
    m_currentVersion = QVersionNumber::fromString(version);
}

