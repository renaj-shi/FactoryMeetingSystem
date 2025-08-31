#pragma once
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QJsonObject>

class SignalingClient : public QObject {
    Q_OBJECT
public:
    explicit SignalingClient(QObject* parent=nullptr);
    void connectToServer(const QUrl& url);
    void sendJson(const QJsonObject& obj);
    void close();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void jsonReceived(const QJsonObject& obj);

private slots:
    void onTextMessage(const QString& text);

private:
    QWebSocket ws_;
};
