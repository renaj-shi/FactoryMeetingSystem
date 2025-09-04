#include "ffmpegrecorder.h"
#include <QStandardPaths>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QFileInfo>
#include <QtGlobal>

FFmpegRecorder::FFmpegRecorder(QObject *parent) : QObject(parent) {
    connect(&m_proc, SIGNAL(readyReadStandardError()),
            this, SLOT(onReadyReadStderr()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(&m_proc, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onProcessError(QProcess::ProcessError)));
#else
    connect(&m_proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onProcessError(QProcess::ProcessError)));
#endif
    connect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onFinished(int,QProcess::ExitStatus)));
}

QString FFmpegRecorder::recordingsRoot() const {
    QString base = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (base.isEmpty()) base = QDir::homePath();
    return base + "/factory_assist/records";
}

QString FFmpegRecorder::detectFfmpegPath() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    const QString p = QStandardPaths::findExecutable("ffmpeg");
    if (!p.isEmpty()) return p;
#endif
    if (QFileInfo::exists("/usr/bin/ffmpeg")) return "/usr/bin/ffmpeg";
    return "ffmpeg";
}

bool FFmpegRecorder::ffmpegSupportsPipewire() const {
    QProcess p;
    p.start(detectFfmpegPath(), QStringList() << "-hide_banner" << "-loglevel" << "quiet" << "-devices");
    if (!p.waitForStarted(2000)) return false;
    p.waitForFinished(3000);
    const QString out = QString::fromUtf8(p.readAllStandardOutput()) + QString::fromUtf8(p.readAllStandardError());
    return out.contains("pipewire", Qt::CaseInsensitive);
}

FFmpegRecorder::Backend FFmpegRecorder::detectBackend() const {
    const QString sess = qEnvironmentVariable("XDG_SESSION_TYPE").toLower();
    if (sess == "x11") return X11;
    if (sess == "wayland") {
        if (ffmpegSupportsPipewire()) return WaylandPipewire;
        return Unknown;
    }
    return X11;
}

QString FFmpegRecorder::pickFontFile() const {
    const QStringList candidates = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    };
    for (const QString &f : candidates) {
        if (QFileInfo::exists(f)) return f;
    }
    return QString();
}

bool FFmpegRecorder::buildInputArgs(Backend backend, QStringList &args, QString &hintError) {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) { hintError = "无法获取屏幕信息"; return false; }

    const QRect g = screen->geometry();
    const qreal dpr = screen->devicePixelRatio();
    const int width  = qMax(1, int(g.width()  * dpr));
    const int height = qMax(1, int(g.height() * dpr));
    const int x = qMax(0, int(g.x() * dpr));
    const int y = qMax(0, int(g.y() * dpr));
    const QString videoSize = QString("%1x%2").arg(width).arg(height);

    if (backend == X11) {
        const QString display = qEnvironmentVariable("DISPLAY", ":0.0");
        const QString x11Input = QString("%1+%2,%3").arg(display).arg(x).arg(y);
        args << "-f" << "x11grab"
             << "-framerate" << "25"
             << "-video_size" << videoSize
             << "-i" << x11Input;
        return true;
    } else if (backend == WaylandPipewire) {
        args << "-f" << "pipewire"
             << "-framerate" << "25"
             << "-video_size" << videoSize
             << "-i" << "0";
        return true;
    } else {
        hintError = "Wayland 会话下 ffmpeg 未启用 pipewire，无法直接抓屏。请切换 Xorg 会话或安装带 pipewire 的 ffmpeg。";
        return false;
    }
}

void FFmpegRecorder::buildEncodeArgs(QStringList &args, bool overlayText, bool withAudio, const QString &workOrderId) {
    if (withAudio) {
        args << "-f" << "pulse" << "-i" << "default";
    }

    args << "-c:v" << "libx264" << "-preset" << "veryfast" << "-crf" << "23";

    if (withAudio) {
        args << "-c:a" << "aac" << "-b:a" << "128k";
    }

    if (overlayText) {
        const QString font = pickFontFile();
        if (!font.isEmpty()) {
            const QString draw =
                QString("drawtext=fontfile=%1:"
                        "text='WO:%2  %%{localtime\\:%%Y-%%m-%%d %%H\\:%%M\\:%%S}':"
                        "x=10:y=10:fontcolor=white:fontsize=24:box=1:boxcolor=0x00000099")
                    .arg(font, workOrderId);
            args << "-vf" << draw;
        } else {
            emit info("未找到可用字体文件，已禁用水印（安装 fonts-dejavu-core 可启用）。");
        }
    }

    args << "-pix_fmt" << "yuv420p" << "-movflags" << "+faststart";
    args << "-metadata" << QString("workorder_id=%1").arg(workOrderId);
}

bool FFmpegRecorder::start(const QString &workOrderId, bool overlayText, bool withAudio) {
    if (isRecording()) return true;

    const QString root = recordingsRoot();
    QDir().mkpath(root + "/" + workOrderId);
    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_outFile = QString("%1/%2/%3_%4.mp4").arg(root, workOrderId, workOrderId, ts);
    m_workOrderId = workOrderId;

    const Backend backend = detectBackend();
    QStringList args;
    QString hintError;

    args << "-y" << "-hide_banner" << "-loglevel" << "info";
    if (!buildInputArgs(backend, args, hintError)) {
        emit error(hintError);
        return false;
    }
    buildEncodeArgs(args, overlayText, withAudio, workOrderId);

    args << m_outFile;

    const QString ffmpeg = detectFfmpegPath();
    m_proc.start(ffmpeg, args);
    if (!m_proc.waitForStarted(2000)) {
        emit error("无法启动 ffmpeg，请确认已安装并可执行。");
        return false;
    }

    m_startTime = QDateTime::currentDateTime();
    emit started(m_outFile);
    return true;
}

void FFmpegRecorder::stop() {
    if (!isRecording()) return;
    m_proc.write("q");
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    m_proc.closeWriteChannel();
#endif
    if (!m_proc.waitForFinished(5000)) {
        m_proc.terminate();
        if (!m_proc.waitForFinished(2000)) {
            m_proc.kill();
        }
    }
}

void FFmpegRecorder::onReadyReadStderr() {
    const QByteArray data = m_proc.readAllStandardError();
    const QString s = QString::fromUtf8(data);
    static QRegularExpression re(QStringLiteral("time=(\\d+):(\\d+):(\\d+(?:\\.\\d+)?)"));
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    if (it.hasNext()) {
        const QRegularExpressionMatch m = it.peekNext();
        const int h = m.captured(1).toInt();
        const int M = m.captured(2).toInt();
        const double sec = m.captured(3).toDouble();
        int secs = int(h * 3600 + M * 60 + sec);
        emit progressSec(secs);
    }
    if (!s.trimmed().isEmpty()) emit info(s.trimmed());
}

void FFmpegRecorder::onProcessError(QProcess::ProcessError) {
    emit error("ffmpeg 进程错误（启动失败或异常终止），请检查 ffmpeg 与权限。");
}

void FFmpegRecorder::onFinished(int, QProcess::ExitStatus) {
    if (!m_outFile.isEmpty()) emit stopped(m_outFile);
}
