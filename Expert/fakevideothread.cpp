#include "fakevideothread.h"
#include <QDebug>

FakeVideoThread::FakeVideoThread(QObject *parent)
    : QThread(parent), m_stop(false)
{
}

FakeVideoThread::FakeVideoThread(const QImage &frame, QObject *parent)
    : QThread(parent), m_frame(frame), m_stop(false)
{
}

FakeVideoThread::~FakeVideoThread()
{
    stop();
}

void FakeVideoThread::stop()
{
    m_stop = true;
    wait();
}

void FakeVideoThread::setVideoFile(const QString &filePath)
{
    videoFilePath = filePath;
}

void FakeVideoThread::run()
{
    if (!videoFilePath.isEmpty()) {
        // 使用本地视频文件
        mediaPlayer = new QMediaPlayer;
        videoProbe = new QVideoProbe;

        if (videoProbe->setSource(mediaPlayer)) {
            connect(videoProbe, &QVideoProbe::videoFrameProbed,
                    this, &FakeVideoThread::onVideoFrameProbed);
        }

        mediaPlayer->setMedia(QUrl::fromLocalFile(videoFilePath));
        mediaPlayer->play();

        // 保持线程运行
        while (!m_stop) {
            msleep(100);
        }

        mediaPlayer->stop();
        delete mediaPlayer;
        delete videoProbe;
    } else {
        // 如果没有传入帧，生成一张纯色测试图
        if (m_frame.isNull()) {
            m_frame = QImage(640, 480, QImage::Format_RGB32);
            m_frame.fill(Qt::green);
        }

        while (!m_stop) {
            emit frameReady(m_frame);
            msleep(100);   // 10 fps
        }
    }
}

void FakeVideoThread::onVideoFrameProbed(const QVideoFrame &frame)
{
    QVideoFrame cloneFrame(frame);
    if (!cloneFrame.map(QAbstractVideoBuffer::ReadOnly)) {
        qWarning() << "Failed to map video frame";
        return;
    }

    QImage image(
        cloneFrame.bits(),
        cloneFrame.width(),
        cloneFrame.height(),
        QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat())
        );

    if (!image.isNull()) {
        emit frameReady(image.copy()); // 必须 copy，否则 frame unmap 后数据失效
    }

    cloneFrame.unmap();
}
