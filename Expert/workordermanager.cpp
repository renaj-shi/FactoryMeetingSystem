// ===================================================================
// [MOD] 文件名: workordermanager.cpp
// [MOD] 内容: 修正了 onLine() 函数中对 "ORDER|JOIN|OK" 响应的处理逻辑，
// [MOD]       确保能够解析工单标题并通过 joined 信号一同发送出去。
// ===================================================================
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
    tcp_->sendLine(QString("ORDER|JOIN|%1|%2").arg(orderId, user));
}

void WorkOrderManager::leave() {
    if (currentOrderId_.isEmpty() || !tcp_) return;
    const QString oid = currentOrderId_;
    tcp_->sendLine(QString("ORDER|LEAVE|%1").arg(oid));
    currentOrderId_.clear();
    emit left(oid);
}

void WorkOrderManager::onLine(const QString& line) {
    if (!line.startsWith("ORDER|")) return;

    const int p0 = line.indexOf('|');
    const int p1 = line.indexOf('|', p0 + 1);
    if (p0 < 0 || p1 < 0) return;

    const QString sub = line.mid(p0 + 1, p1 - (p0 + 1));

    if (sub == "INVITE") {
        const int p2 = line.indexOf('|', p1 + 1);
        if (p2 < 0) return;
        const QString oid   = line.mid(p1 + 1, p2 - (p1 + 1));
        const QString title = line.mid(p2 + 1);
        emit inviteReceived(oid, title);
        return;
    }

    if (sub == "JOIN") {
        // [MOD] 假设服务器成功时返回格式为：ORDER|JOIN|OK|orderId|title
        const int p2 = line.indexOf('|', p1 + 1);
        if (p2 < 0) return;
        const QString st   = line.mid(p1 + 1, p2 - (p1 + 1));
        const QString tail = line.mid(p2 + 1);

        if (st == "OK") {
            QString oid = tail;
            QString title; // [ADD] 新增 title 变量
            const int p3 = tail.indexOf('|');
            if (p3 >= 0) {
                // [MOD] 如果找到了'|'，则分别解析ID和标题
                oid = tail.left(p3);
                title = tail.mid(p3 + 1);
            }
            currentOrderId_ = oid;
            // [MOD] 发射带有ID和标题的信号
            emit joined(oid, title);
        } else if (st == "ERR") {
            const QString reason = tail;
            emit joinFailed(reason.isEmpty() ? tr("未知错误") : reason);
        }
        return;
    }

    if (sub == "LEAVE") {
        return;
    }

    if (sub == "CLOSED") {
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
