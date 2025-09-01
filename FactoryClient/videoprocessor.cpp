// videoprocessor.cpp
#include "videoprocessor.h"
#include <QBuffer>
#include <QByteArray>
#include <QImageWriter>
#include <QDebug>
#include <QCameraInfo>
#include <QCameraViewfinder>

VideoProcessor::VideoProcessor(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_viewfinder(nullptr)
    , m_imageCapture(nullptr)
    , m_captureTimer(nullptr)
    , m_isCapturing(false)
    , m_shouldCapture(false)
{
    initializeCamera();
}

VideoProcessor::~VideoProcessor()
{
    stopCapture();

    // 注意：需要先删除依赖相机的对象
    if (m_imageCapture) {
        delete m_imageCapture;
        m_imageCapture = nullptr;
    }

    if (m_viewfinder) {
        m_viewfinder->deleteLater(); // 使用 deleteLater 因为它是 QWidget
        m_viewfinder = nullptr;
    }

    if (m_camera) {
        delete m_camera;
        m_camera = nullptr;
    }

    if (m_captureTimer) {
        delete m_captureTimer;
        m_captureTimer = nullptr;
    }
}

void VideoProcessor::initializeCamera()
{
    // 检查是否有可用摄像头
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        qDebug() << "No camera available";
        return;
    }

    // 创建摄像头对象
    m_camera = new QCamera(cameras.first());

    // 创建取景器（注意：不能设置 QObject 作为父对象）
    m_viewfinder = new QCameraViewfinder();
    m_camera->setViewfinder(m_viewfinder);

    // 创建图像捕获对象
    m_imageCapture = new QCameraImageCapture(m_camera);
    m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    // 连接图像捕获信号
    connect(m_imageCapture, &QCameraImageCapture::imageCaptured,
            this, &VideoProcessor::onImageCaptured);

    // 创建捕获定时器
    m_captureTimer = new QTimer(this); // 这个可以设置父对象，因为它是 QObject
    m_captureTimer->setInterval(100); // 每100ms捕获一帧
    connect(m_captureTimer, &QTimer::timeout, this, &VideoProcessor::onCaptureTimer);

    // 设置摄像头参数
    m_camera->setCaptureMode(QCamera::CaptureVideo);
}

void VideoProcessor::startCapture()
{
    if (m_isCapturing || !m_camera) {
        return;
    }

    m_camera->start();
    m_captureTimer->start();
    m_isCapturing = true;
    m_shouldCapture = true;
}

void VideoProcessor::stopCapture()
{
    if (!m_isCapturing || !m_camera) {
        return;
    }

    m_captureTimer->stop();
    m_camera->stop();
    m_isCapturing = false;
    m_shouldCapture = false;
}

void VideoProcessor::displayVideo(bool display)
{
    if (m_viewfinder) {
        m_viewfinder->setVisible(display);
    }
}

bool VideoProcessor::isCapturing() const
{
    return m_isCapturing;
}

QCameraViewfinder* VideoProcessor::getViewfinder()
{
    return m_viewfinder;
}

void VideoProcessor::onCaptureTimer()
{
    if (m_shouldCapture && m_imageCapture && m_camera->state() == QCamera::ActiveState) {
        m_imageCapture->capture();
    }
}

void VideoProcessor::onImageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id);

    // 将图像转换为字节数组
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);

    // 压缩图像以减少数据量
    QImage scaledImage = preview.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledImage.save(&buffer, "JPEG", 50); // 50% 质量压缩

    emit videoFrameReady(imageData);
}
