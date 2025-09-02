#include "videothread.h"
#include <QCameraImageCapture>
#include <QElapsedTimer>
#include <QThread>

VideoThread::VideoThread(QCameraImageCapture *capture, QObject *parent)
    : QThread(parent), imageCapture(capture), stopped(false)
{
}

VideoThread::~VideoThread()
{
    stop();
    wait();
}

void VideoThread::stop()
{
    stopped = true;
}

void VideoThread::run()
{
    QElapsedTimer timer;
    timer.start();

    while (!stopped) {
        if (timer.elapsed() >= 100) { // 每100ms抓一帧
            if (imageCapture) {
                imageCapture->capture();
            }
            timer.restart();
        }
        QThread::msleep(10);
    }
}
