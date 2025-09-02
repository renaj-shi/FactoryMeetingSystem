#ifndef FAKEVIDEOTHREAD_H
#define FAKEVIDEOTHREAD_H

#include <QThread>
#include <QImage>
#include <atomic>
#include <QMediaPlayer>
#include <QVideoProbe>

class FakeVideoThread : public QThread
{
    Q_OBJECT

public:
    explicit FakeVideoThread(QObject *parent = nullptr);
    explicit FakeVideoThread(const QImage &frame, QObject *parent = nullptr);
    ~FakeVideoThread();

    void stop();
    void setVideoFile(const QString &filePath);

signals:
    void frameReady(const QImage &img);

protected:
    void run() override;

private slots:
    void onVideoFrameProbed(const QVideoFrame &frame);

private:
    QImage m_frame;
    std::atomic_bool m_stop;
    QMediaPlayer *mediaPlayer;
    QVideoProbe *videoProbe;
    QString videoFilePath;
};

#endif // FAKEVIDEOTHREAD_H
