#pragma once

#include <QObject>
#include <QDebug>

#include "arguments.h"
#include "manifest.h"
#include "downloader.h"
#include "manifestfile.h"
#include "latestresponse.h"

class Updater : public QObject
{
    Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged FINAL)
    Q_PROPERTY(float progress READ progress NOTIFY progressChanged FINAL)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged FINAL)
    Q_PROPERTY(QString log READ log NOTIFY logChanged FINAL)

public:
    explicit Updater(QObject *parent = nullptr);

    void start();

    Q_INVOKABLE void skipUpdate();
    Q_INVOKABLE void confirmUpdate();

    enum class State
    {
        Unknown=0,
        Idle,
        ParsingArguments,
        DownloadingLatestResponse,
        DownloadingManifest,
        ComparingFiles,
        WaitForConfirmation,
        DownloadingFiles,
        VerifyingFiles,
        Installing,
        Launching,
        Finished,
        Error
    };
    Q_ENUM(State)

    //getter and setters
    State state() const;
    void setState(State state);

    QString title() const;
    void setTitle(const QString &newTitle);

    QString description() const;
    void setDescription(const QString &newDescription);

    float progress() const;
    void setProgress(float newProgress);

    bool busy() const;
    void setBusy(bool newBusy);

    QString log() const;
    void setLog(const QString &newLog);
    void appendLog(const QString &newLog);

signals:

    void finished();

    void errorOccurred(const QString &message);

    void stateChanged();
    void titleChanged();

    void descriptionChanged();

    void progressChanged();

    void busyChanged();

    void logChanged();

private:
    void startRepair();
    void compareLocalFiles();
    void verifyDownloadedFiles();
    void installFiles();
    void launchApplication();

    Arguments m_arguments;
    LatestResponse m_latestResponse;
    Manifest m_manifest;
    Downloader m_downloader;
    QVector<ManifestFile> m_downloadQueue;

    //path
    QString m_tempDirectory;
    QString m_installDirectory;


    //
    State m_state = State::Idle;
    QString m_title;
    QString m_description;
    float m_progress = 0.0f;
    bool m_busy = false;
    QString m_log;

    //download state
    int m_currentDownload = 0;
    QByteArray m_currentDownloadedData;
    void downloadNextFile();
    void fileDownloaded(const QByteArray &data);
};
