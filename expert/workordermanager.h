#pragma once
#include <QObject>
#include <QString>

class TcpClient;
class AuthManager;

class WorkOrderManager : public QObject {
    Q_OBJECT
public:
    explicit WorkOrderManager(TcpClient* tcp, AuthManager* auth, QObject* parent=nullptr);
    void join(const QString& orderId);  // ORDER|JOIN|orderId|user
    void leave();                       // ORDER|LEAVE|orderId

    QString currentOrderId() const { return currentOrderId_; }

signals:
    void inviteReceived(const QString& orderId, const QString& title);
    void joined(const QString& orderId);
    void joinFailed(const QString& reason);
    void left(const QString& orderId);
    void closed(const QString& orderId, const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
    AuthManager* auth_{nullptr};
    QString currentOrderId_;
};
