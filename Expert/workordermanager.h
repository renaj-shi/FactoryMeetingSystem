// ===================================================================
// [MOD] 文件名: workordermanager.h
// [MOD] 内容: 确认 joined 信号的声明是正确的，包含 orderId 和 title 两个参数。
// ===================================================================
#pragma once
#include <QObject>
#include <QString>

class TcpClient;
class AuthManager;

class WorkOrderManager : public QObject {
    Q_OBJECT
public:
    explicit WorkOrderManager(TcpClient* tcp, AuthManager* auth, QObject* parent=nullptr);
    void join(const QString& orderId);
    void leave();

    QString currentOrderId() const { return currentOrderId_; }

signals:
    void inviteReceived(const QString& orderId, const QString& title);
    // [MOD] 确认信号签名包含ID和标题
    void joined(const QString& orderId, const QString& title);
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
