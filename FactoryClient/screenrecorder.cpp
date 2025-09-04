#include "screenrecorder.h"
#include <QStandardPaths>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QFileInfo>
#include <QtGlobal>
#include <QString>
#include <QDebug>

ScreenRecorder::ScreenRecorder(QObject *parent) : QObject(parent) {
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

QString ScreenRecorder::recordingsRoot() const {
    // 对于Windows环境，使用文档目录
    QString base = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (base.isEmpty()) base = QDir::homePath();
    return base + "/factory_assist/records";
}

QString ScreenRecorder::detectFfmpegPath() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    const QString p = QStandardPaths::findExecutable("ffmpeg");
    if (!p.isEmpty()) return p;
#endif
    // Windows环境下常见的ffmpeg路径
    if (QFileInfo::exists("C:/ffmpeg/bin/ffmpeg.exe")) return "C:/ffmpeg/bin/ffmpeg.exe";
    if (QFileInfo::exists("C:/Program Files/ffmpeg/bin/ffmpeg.exe")) return "C:/Program Files/ffmpeg/bin/ffmpeg.exe";
    
    // 如果找不到，尝试直接使用ffmpeg命令（假设已添加到系统路径）
    return "ffmpeg";
}

ScreenRecorder::Backend ScreenRecorder::detectBackend() const {
#ifdef Q_OS_WIN
    return WindowsGDI;
#else
    const QString sess = qEnvironmentVariable("XDG_SESSION_TYPE").toLower();
    if (sess == "x11") return X11;
    if (sess == "wayland") {
        // Wayland环境下的pipewire支持检查
        QProcess p;
        p.start(detectFfmpegPath(), QStringList() << "-hide_banner" << "-loglevel" << "quiet" << "-devices");
        if (!p.waitForStarted(2000)) return Unknown;
        p.waitForFinished(3000);
        const QString out = QString::fromUtf8(p.readAllStandardOutput()) + QString::fromUtf8(p.readAllStandardError());
        if (out.contains("pipewire", Qt::CaseInsensitive)) return WaylandPipewire;
        return Unknown;
    }
    return X11;
#endif
}

QString ScreenRecorder::pickFontFile() const {
#ifdef Q_OS_WIN
    // Windows环境下常见的字体路径
    const QStringList candidates = {
        "C:/Windows/Fonts/simhei.ttf", // 黑体
        "C:/Windows/Fonts/simsun.ttc", // 宋体
        "C:/Windows/Fonts/micross.ttf", // Microsoft Sans Serif
        "C:/Windows/Fonts/arial.ttf"     // Arial
    };
#else
    // Linux环境下的字体路径
    const QStringList candidates = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    };
#endif
    for (const QString &f : candidates) {
        if (QFileInfo::exists(f)) return f;
    }
    return QString();
}

bool ScreenRecorder::buildInputArgs(Backend backend, QStringList &args, QString &hintError) {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) { hintError = "无法获取屏幕信息"; return false; }

    const QRect g = screen->geometry();
    const qreal dpr = screen->devicePixelRatio();
    const int width  = qMax(1, int(g.width()  * dpr));
    const int height = qMax(1, int(g.height() * dpr));
    const QString videoSize = QString("%1x%2").arg(width).arg(height);

    if (backend == WindowsGDI) {
        // Windows GDI抓屏
        args << "-f" << "gdigrab"
             << "-framerate" << "25"
             << "-video_size" << videoSize
             << "-i" << "desktop";
        return true;
    } else if (backend == X11) {
        const QString display = qEnvironmentVariable("DISPLAY", ":0.0");
        // 修正坐标计算，使其也考虑dpr
        const int x = int(g.x() * dpr);
        const int y = int(g.y() * dpr);
        const QString x11Input = QString("%1+%2,%3").arg(display).arg(x).arg(y);
        
        // 尝试使用x11grab作为输入格式
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
        hintError = "当前环境不支持屏幕录制，请确认操作系统和ffmpeg配置。";
        return false;
    }
}

void ScreenRecorder::buildEncodeArgs(QStringList &args, bool overlayText, bool withAudio, const QString &workOrderId) {
    if (withAudio) {
#ifdef Q_OS_WIN
        // Windows环境下的音频输入
        args << "-f" << "dshow" << "-i" << "audio=麦克风阵列 (Realtek(R) Audio)";
#else
        // Linux环境下的音频输入，先尝试使用pulse，如果失败会自动回退到无音频
        // 添加ignore_unknown和strict实验性功能，提高兼容性
        args << "-f" << "pulse" << "-i" << "default"
             << "-ignore_unknown" << "-strict" << "experimental";
#endif
    }

    // 视频和音频编码参数
    args << "-c:v" << "libx264" << "-preset" << "veryfast" << "-crf" << "23";

    if (withAudio) {
        args << "-c:a" << "aac" << "-b:a" << "128k";
    }

    // 添加水印文本
    if (overlayText) {
        const QString font = pickFontFile();
        if (!font.isEmpty()) {
            const QString draw = 
                QString("drawtext=fontfile=%1:"
                        "text='工单:%2  %%{localtime\\:%%Y-%%m-%%d %%H\\:%%M\\:%%S}':"
                        "x=10:y=10:fontcolor=white:fontsize=24:box=1:boxcolor=0x00000099")
                    .arg(font, workOrderId);
            args << "-vf" << draw;
        } else {
            emit info("未找到可用字体文件，已禁用水印。");
        }
    }

    // 输出格式设置
    args << "-pix_fmt" << "yuv420p" << "-movflags" << "+faststart";
    args << "-metadata" << QString("workorder_id=%1").arg(workOrderId);
}

bool ScreenRecorder::start(const QString &workOrderId, bool overlayText, bool withAudio) {
    if (isRecording()) return true;

    // 创建录制目录
    const QString root = recordingsRoot();
    QDir().mkpath(root + "/" + workOrderId);
    const QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_outFile = QString("%1/%2/%3_%4.mp4").arg(root, workOrderId, workOrderId, ts);
    m_workOrderId = workOrderId;

    // 检测后端并构建命令参数
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

    // 启动ffmpeg进程
    const QString ffmpeg = detectFfmpegPath();
    qDebug() << "Starting ffmpeg with command:" << ffmpeg << args.join(" ");
    m_proc.start(ffmpeg, args);
    if (!m_proc.waitForStarted(2000)) {
        emit error("无法启动ffmpeg，请确认已安装并可执行。");
        return false;
    }

    m_startTime = QDateTime::currentDateTime();
    emit started(m_outFile);
    return true;
}

void ScreenRecorder::stop() {
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

void ScreenRecorder::onReadyReadStderr() {
    const QByteArray data = m_proc.readAllStandardError();
    const QString s = QString::fromUtf8(data);
    // 解析输出，更新进度
    // 改进正则表达式，使其更加健壮，能够处理不同情况下的ffmpeg输出
    static QRegularExpression re(QStringLiteral("\\btime=\\s*(\\d+):(\\d+):(\\d+(?:\\.\\d+)?)") , QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    if (it.hasNext()) {
        const QRegularExpressionMatch m = it.peekNext();
        const int h = m.captured(1).toInt();
        const int M = m.captured(2).toInt();
        const double sec = m.captured(3).toDouble();
        int secs = int(h * 3600 + M * 60 + sec);
        emit progressSec(secs);
    }
    
    // 增加音频设备错误检测和提示
    if (s.contains("Unknown input format", Qt::CaseInsensitive) && s.contains("x11grab", Qt::CaseInsensitive)) {
        emit error("ffmpeg不支持x11grab格式，请确认ffmpeg版本是否支持X11录制");
    }
    if (s.contains("Connection refused", Qt::CaseInsensitive) && s.contains("pulse", Qt::CaseInsensitive)) {
        emit error("无法连接到音频设备，请检查PulseAudio服务是否运行");
    }
    
    if (!s.trimmed().isEmpty()) emit info(s.trimmed());
}

void ScreenRecorder::onProcessError(QProcess::ProcessError) {
    emit error("ffmpeg进程错误（启动失败或异常终止），请检查ffmpeg与权限。");
}

void ScreenRecorder::onFinished(int, QProcess::ExitStatus) {
    if (!m_outFile.isEmpty()) emit stopped(m_outFile);
}