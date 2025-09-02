#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
#include <QImage>

class QCameraImageCapture;

class VideoThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoThread(QCameraImageCapture *capture, QObject *parent = nullptr);
    ~VideoThread();
    void stop();

signals:
    void frameCaptured(QImage image);

protected:
    void run() override;

private:
    QCameraImageCapture *imageCapture;
    volatile bool stopped;
};

#endif // VIDEOTHREAD_H
