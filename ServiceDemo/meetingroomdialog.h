#ifndef MEETINGROOMDIALOG_H
#define MEETINGROOMDIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QMap>

namespace Ui {
class MeetingRoomDialog;
}

class MeetingRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeetingRoomDialog(QWidget *parent = nullptr);
    ~MeetingRoomDialog();

    bool startMeetingServer(int port = 12347);
    void stopMeetingServer();

signals:
    void meetingClosed();

public slots:
    void addSystemMessage(const QString &message);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

    void on_sendButton_clicked();
    void on_broadcastButton_clicked();
    void on_kickButton_clicked();
    void on_closeMeetingButton_clicked();
    void on_startMeetingButton_clicked();
    void on_sendImageButton_clicked();
    void sendImageFromServer(const QString &imagePath);
    void handleAudioData(QTcpSocket* socket, const QByteArray& data);

private:
    Ui::MeetingRoomDialog *ui;
    QTcpServer *meetingServer;
    QSet<QTcpSocket*> meetingClients;
    QMap<QTcpSocket*, QString> clientUsers;
    QMap<QTcpSocket*, QString> clientTypes;
    QMap<QString, QTcpSocket*> userSockets;

    void handleChatMessage(QTcpSocket* socket, const QJsonObject& json);
    void handleImageData(QTcpSocket* socket, const QByteArray& data);
    void broadcastMessage(const QJsonObject& json, QTcpSocket* exclude = nullptr);
    void broadcastToAll(const QByteArray& data, const QString& type = "", QTcpSocket* exclude = nullptr);
    void updateClientList();
    void addChatMessage(const QString& user, const QString& message, const QString& time);
};

#endif // MEETINGROOMDIALOG_H
