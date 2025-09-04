#pragma once
#include <QObject>

class TcpClient;
class AuthManager;

class ChatClient : public QObject {
    Q_OBJECT
public:
    explicit ChatClient(TcpClient* tcp, AuthManager* auth, QObject* parent=nullptr);
    void sendText(const QString& orderId, const QString& text);

signals:
    void messageReceived(const QString& orderId, const QString& from, const QString& text, qint64 ts);
    void sendFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
    AuthManager* auth_{nullptr};
};
