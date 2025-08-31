#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>

class SignalingClient;

class ChatService : public QObject {
    Q_OBJECT
public:
    explicit ChatService(SignalingClient* sig, QObject* parent=nullptr);
    void sendText(const QString& orderId, const QString& text);

signals:
    void messageReceived(const QString& orderId, const QString& from, const QString& text, qint64 ts);

private slots:
    void handleMsg(const QJsonObject& obj);

private:
    SignalingClient* sig_;
};
