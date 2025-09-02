#ifndef MEETINGROOMDIALOG_H
#define MEETINGROOMDIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QMap>
#include <QJsonObject>

// 添加多媒体支持
#ifdef QT_MULTIMEDIA_LIB
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include "videothread.h"  // 您的VideoThread类
#endif

namespace Ui {
class MeetingRoomDialog;
}

class MeetingRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeetingRoomDialog(QWidget *parent = nullptr);
    ~MeetingRoomDialog();

    bool startMeetingServer(int port);
    void stopMeetingServer();

signals:
    void meetingClosed();

private slots:
    // 原有槽函数...
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void on_sendButton_clicked();
    void on_broadcastButton_clicked();
    void on_kickButton_clicked();
    void on_closeMeetingButton_clicked();
    void on_startMeetingButton_clicked();

    // 新增视频功能槽函数
    void on_startVideoButton_clicked();
    void on_stopVideoButton_clicked();
    void videoFrameReceived(QImage image);

    void on_sendImageButton_clicked();
    void sendImageFromServer(const QString &imagePath);
    void handleAudioData(QTcpSocket* socket, const QByteArray& data);

    private:
    // 原有私有函数...
    void addSystemMessage(const QString& message);
    void handleChatMessage(QTcpSocket* socket, const QJsonObject& json);
    void handleImageData(QTcpSocket* socket, const QByteArray& data);
    void broadcastMessage(const QJsonObject& json, QTcpSocket* exclude = nullptr);
    void broadcastToAll(const QByteArray& data, const QString& type = "", QTcpSocket* exclude = nullptr);
    void updateClientList();
    void addChatMessage(const QString& user, const QString& message, const QString& time);

    // 新增视频功能私有函数
    void releaseCameraResources();
    void updateUIStatus();
    void handleVideoFrame(const QJsonObject &json);
#ifdef QT_MULTIMEDIA_LIB
    void sendVideoFrame(const QImage &image);
#endif

private:
    Ui::MeetingRoomDialog *ui;
    QTcpServer *meetingServer;
    QSet<QTcpSocket*> meetingClients;
    QMap<QTcpSocket*, QString> clientUsers;
    QMap<QTcpSocket*, QString> clientTypes;
    QMap<QString, QTcpSocket*> userSockets;

    // 新增视频功能成员变量
#ifdef QT_MULTIMEDIA_LIB
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;
    VideoThread *videoThread;
#endif
};

#endif // MEETINGROOMDIALOG_H
