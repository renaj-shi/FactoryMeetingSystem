#ifndef MEETINGDIALOG_H
#define MEETINGDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QString>
#include <QJsonObject>
#include <QImage>
#include <QLabel>
#include "audioprocessor.h"
#ifdef QT_MULTIMEDIA_LIB
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#endif

#include "fakevideothread.h"

class QTextEdit;
class QLineEdit;
class QPushButton;
           // 前置声明
class MeetingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeetingDialog(const QString &username,
                           const QString &server,
                           int port,  // 改为 int 而不是 quint16
                           const QString &meetingId = "",
                           QWidget *parent = nullptr);
    ~MeetingDialog();

signals:
    void meetingClosed();

private slots:
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onJoinMeetingClicked();
    void onLeaveMeetingClicked();
    void onSendButtonClicked();
    void onSocketReadyRead();
    void onSocketDisconnected();
    // 新增视频功能槽函数
    void onStartVideoClicked();
    void onStopVideoClicked();
    void onVideoFrameReceived(const QImage &frame);
    //音频槽函数
    void onSendImageClicked(); // 新增发送图片槽函数
    void onAudioButtonClicked();
    void onAudioDataReady(const QByteArray &audioData);

private:
    void setupUI();
    void sendChatMessage(const QString &message);
    void sendJoinMeeting();
    void sendLeaveMeeting();
    void handleServerMessage(const QJsonObject &json);
    void addMessageToChat(const QString &sender, const QString &message);
    void processVideoFrame(const QJsonObject &json);
    void sendImageMessage(const QString &imagePath); // 新增发送图片方法

    // 视频相关功能
    void startVideo();
    void stopVideo();
    void sendVideoFrame(const QImage &frame);
    void setupVideoUI();

    QString username;
    bool isInMeeting;
    QTcpSocket *tcpSocket;
    AudioProcessor *audioProcessor;
    QPushButton *audioButton;
    bool isAudioEnabled;
    // UI components
    QTextEdit *chatTextEdit;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QPushButton *joinMeetingButton;
    QPushButton *leaveMeetingButton;
    QPushButton *sendImageButton; // 新增发送图片按钮

    // 新增视频UI组件
    QLabel *localVideoLabel;
    QLabel *remoteVideoLabel;
    QPushButton *startVideoButton;
    QPushButton *stopVideoButton;
    QWidget *videoContainer;
    FakeVideoThread *fakeVideoThread = nullptr;;
};

#endif // MEETINGDIALOG_H
