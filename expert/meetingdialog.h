#ifndef MEETINGDIALOG_H
#define MEETINGDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QString>
#include <QLabel>
#include "fakevideothread.h"
#include "audioprocessor.h"

class QTextEdit;
class QLineEdit;
class QPushButton;

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
    void onSendImageClicked();
    void sendImageMessage(const QString &imagePath);
    void onAudioButtonClicked();
    void onAudioDataReady(const QByteArray &audioData);


private:
    void setupUI();
    void sendChatMessage(const QString &message);
    void sendJoinMeeting();
    void sendLeaveMeeting();
    void handleServerMessage(const QJsonObject &json);
    void addMessageToChat(const QString &sender, const QString &message);
    void setupVideoUI();           // 新增
    void startVideo();             // 新增
    void stopVideo();
    void sendVideoFrame(const QImage &frame);
    void onVideoFrameReceived(const QImage &frame);
    void processVideoFrame(const QJsonObject &json);
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
    QLabel *localVideoLabel  = nullptr;
    QLabel *remoteVideoLabel = nullptr;
    QPushButton *startVideoButton = nullptr;
    QPushButton *stopVideoButton  = nullptr;
    FakeVideoThread *fakeVideoThread = nullptr;
    QWidget *videoContainerWidget;
    QPushButton *sendImageButton;
};

#endif // MEETINGDIALOG_H
