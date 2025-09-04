#pragma once
#include <QObject>
#include <QProcess>
#include <QDateTime>

class ScreenRecorder : public QObject {
    Q_OBJECT
public:
    explicit ScreenRecorder(QObject *parent = nullptr);

    bool start(const QString &workOrderId, bool overlayText = true, bool withAudio = true);
    void stop();
    bool isRecording() const { return m_proc.state() == QProcess::Running; }
    QString lastOutputFile() const { return m_outFile; }

signals:
    void started(const QString &filePath);
    void stopped(const QString &filePath);
    void error(const QString &msg);
    void progressSec(int seconds);
    void info(const QString &msg);

private slots:
    void onReadyReadStderr();
    void onProcessError(QProcess::ProcessError e);
    void onFinished(int exitCode, QProcess::ExitStatus status);

private:
    enum Backend { X11, WaylandPipewire, WindowsGDI, Unknown };

    QString recordingsRoot() const;
    QString detectFfmpegPath() const;
    Backend detectBackend() const;
    QString pickFontFile() const;

    bool buildInputArgs(Backend backend, QStringList &args, QString &hintError);
    void buildEncodeArgs(QStringList &args, bool overlayText, bool withAudio, const QString &workOrderId);

private:
    QProcess m_proc;
    QString  m_outFile;
    QString  m_workOrderId;
    QDateTime m_startTime;
};