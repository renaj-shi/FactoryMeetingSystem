// videoprocessor.h
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QTimer>
#include <QImage>

class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(QObject *parent = nullptr);
    ~VideoProcessor();

    void startCapture();
    void stopCapture();
    void displayVideo(bool display);
    bool isCapturing() const;

    QCameraViewfinder* getViewfinder();

signals:
    void videoFrameReady(const QByteArray &frameData);

private slots:
    void onCaptureTimer();
    void onImageCaptured(int id, const QImage &preview);

private:
    void initializeCamera();

    QCamera *m_camera;
    QCameraViewfinder *m_viewfinder;
    QCameraImageCapture *m_imageCapture;
    QTimer *m_captureTimer;
    bool m_isCapturing;
    bool m_shouldCapture;
};

#endif // VIDEOPROCESSOR_H
