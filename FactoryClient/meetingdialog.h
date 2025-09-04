#ifndef MEETINGDIALOG_H
#define MEETINGDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QString>
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
    void sendImageMessage(const QString &imagePath); // 新增发送图片方法

    QString username;
    QString serverHost_;
    int serverPort_ = 0;
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

};

#endif // MEETINGDIALOG_H
