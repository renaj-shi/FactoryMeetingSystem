#include "chatservice.h"
#include "signalingclient.h"
#include <QJsonObject>
#include <QVariant>

ChatService::ChatService(SignalingClient* sig, QObject* parent)
    : QObject(parent), sig_(sig) {
    connect(sig_, &SignalingClient::jsonReceived, this, &ChatService::handleMsg);
}

void ChatService::sendText(const QString& orderId, const QString& text) {
    if (orderId.isEmpty() || text.isEmpty()) return;
    QJsonObject obj{{"type","chat.send"},{"orderId",orderId},{"text",text}};
    sig_->sendJson(obj);
}

void ChatService::handleMsg(const QJsonObject& obj) {
    const auto t = obj.value("type").toString();
    if (t=="chat.msg") {
        const QString orderId = obj.value("orderId").toString();
        const QString from    = obj.value("from").toString();
        const QString text    = obj.value("text").toString();
        const qint64 ts       = obj.value("ts").toVariant().toLongLong();
        emit messageReceived(orderId, from, text, ts);
    }
}
