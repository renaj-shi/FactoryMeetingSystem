#include "workordermanager.h"
#include "tcpclient.h"
#include "authmanager.h"
#include <QStringList>

WorkOrderManager::WorkOrderManager(TcpClient* tcp, AuthManager* auth, QObject* parent)
    : QObject(parent), tcp_(tcp), auth_(auth) {
    connect(tcp_, &TcpClient::lineReceived, this, &WorkOrderManager::onLine);
}

void WorkOrderManager::join(const QString& orderId) {
    if (!tcp_ || !tcp_->isConnected()) {
        emit joinFailed(tr("未连接服务器"));
        return;
    }
    if (!auth_ || auth_->userId().isEmpty()) {
        emit joinFailed(tr("未登录"));
        return;
    }
    const QString user = auth_->userId();
    // 按协议：ORDER|JOIN|orderId|expertUser
    tcp_->sendLine(QString("ORDER|JOIN|%1|%2").arg(orderId, user));
}

void WorkOrderManager::leave() {
    if (currentOrderId_.isEmpty() || !tcp_) return;
    const QString oid = currentOrderId_;
    // 方案A：仅发送指令，立即本地生效并发出 left（不等待服务端 ACK）
    tcp_->sendLine(QString("ORDER|LEAVE|%1").arg(oid));
    currentOrderId_.clear();
    emit left(oid);
}

void WorkOrderManager::onLine(const QString& line) {
    if (!line.startsWith("ORDER|")) return;

    // ORDER|<sub>|...
    const int p0 = line.indexOf('|');
    const int p1 = line.indexOf('|', p0 + 1);
    if (p0 < 0 || p1 < 0) return;

    const QString sub = line.mid(p0 + 1, p1 - (p0 + 1)); // INVITE/JOIN/LEAVE/CLOSED

    if (sub == "INVITE") {
        // ORDER|INVITE|orderId|title...
        const int p2 = line.indexOf('|', p1 + 1);
        if (p2 < 0) return;
        const QString oid   = line.mid(p1 + 1, p2 - (p1 + 1));
        const QString title = line.mid(p2 + 1);
        emit inviteReceived(oid, title);
        return;
    }

    if (sub == "JOIN") {
        // ORDER|JOIN|OK|orderId  或  ORDER|JOIN|ERR|reason...
        const int p2 = line.indexOf('|', p1 + 1);
        if (p2 < 0) return;
        const QString st   = line.mid(p1 + 1, p2 - (p1 + 1)); // OK / ERR
        const QString tail = line.mid(p2 + 1);

        if (st == "OK") {
            QString oid = tail;
            const int p3 = tail.indexOf('|');
            if (p3 >= 0) oid = tail.left(p3); // 容错
            currentOrderId_ = oid;
            emit joined(oid);
        } else if (st == "ERR") {
            const QString reason = tail;
            emit joinFailed(reason.isEmpty() ? tr("未知错误") : reason);
        }
        return;
    }

    if (sub == "LEAVE") {
        // 方案A：忽略服务端的 LEAVE 确认，避免重复 left
        // 格式通常为 ORDER|LEAVE|OK|orderId，这里直接返回
        return;
    }

    if (sub == "CLOSED") {
        // ORDER|CLOSED|orderId|reason...
        const int p2 = line.indexOf('|', p1 + 1);
        if (p2 < 0) return;
        const int p3 = line.indexOf('|', p2 + 1);
        if (p3 < 0) return;

        const QString oid    = line.mid(p2 + 1, p3 - (p2 + 1));
        const QString reason = line.mid(p3 + 1);

        if (currentOrderId_ == oid) currentOrderId_.clear();
        emit closed(oid, reason);
        return;
    }
}
