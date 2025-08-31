#include "chatclient.h"
#include "tcpclient.h"
#include "authmanager.h"
#include <QDateTime>

static QString sanitizeText(QString s) {
    s.replace('\n', ' ').replace('\r', ' ');
    s.replace('|', QChar(0xFF5C)); // 全角竖线，避免协议切割
    return s;
}

ChatClient::ChatClient(TcpClient* tcp, AuthManager* auth, QObject* parent)
    : QObject(parent), tcp_(tcp), auth_(auth) {
    connect(tcp_, &TcpClient::lineReceived, this, &ChatClient::onLine);
}

void ChatClient::sendText(const QString& orderId, const QString& text) {
    if (!tcp_ || !tcp_->isConnected()) { emit sendFailed(QObject::tr("未连接服务器")); return; }
    if (!auth_ || auth_->userId().isEmpty()) { emit sendFailed(QObject::tr("未登录")); return; }
    if (orderId.isEmpty()) { emit sendFailed(QObject::tr("未加入工单")); return; }
    const QString from = auth_->userId();
    const QString safe = sanitizeText(text.trimmed());
    if (safe.isEmpty()) return;
    // CHAT|SEND|orderId|from|text
    tcp_->sendLine(QString("CHAT|SEND|%1|%2|%3").arg(orderId, from, safe));
}

void ChatClient::onLine(const QString& line) {
    if (!line.startsWith("CHAT|")) return;

    // CHAT|MSG|orderId|from|ts|text(余下所有)
    int p0 = line.indexOf('|');
    int p1 = line.indexOf('|', p0+1);
    int p2 = line.indexOf('|', p1+1);
    int p3 = line.indexOf('|', p2+1);
    int p4 = line.indexOf('|', p3+1);
    if (p0<0||p1<0||p2<0||p3<0||p4<0) return;

    const QString sub     = line.mid(p0+1, p1-(p0+1));   // MSG
    if (sub != "MSG") return;

    const QString orderId = line.mid(p1+1, p2-(p1+1));
    const QString from    = line.mid(p2+1, p3-(p2+1));
    const QString tsStr   = line.mid(p3+1, p4-(p3+1));
    const QString text    = line.mid(p4+1);

    bool ok=false; qint64 ts = tsStr.toLongLong(&ok);
    if (!ok) ts = QDateTime::currentMSecsSinceEpoch();

    emit messageReceived(orderId, from, text, ts);
}
