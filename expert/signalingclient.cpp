#include "signalingclient.h"
#include <QJsonDocument>
#include <QAbstractSocket>
#include <QDebug>

SignalingClient::SignalingClient(QObject* parent) : QObject(parent) {
    connect(&ws_, &QWebSocket::connected,    this, &SignalingClient::connected);
    connect(&ws_, &QWebSocket::disconnected, this, &SignalingClient::disconnected);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(&ws_, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, [this](auto){ emit errorOccurred(ws_.errorString()); });
#else
    connect(&ws_, &QWebSocket::errorOccurred,
            this, [this](auto){ emit errorOccurred(ws_.errorString()); });
#endif
    connect(&ws_, &QWebSocket::textMessageReceived, this, &SignalingClient::onTextMessage);
}

void SignalingClient::connectToServer(const QUrl& url) { ws_.open(url); }

void SignalingClient::sendJson(const QJsonObject& obj) {
    const auto msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    ws_.sendTextMessage(QString::fromUtf8(msg));
}

void SignalingClient::close() { ws_.close(); }

void SignalingClient::onTextMessage(const QString& text) {
    const auto doc = QJsonDocument::fromJson(text.toUtf8());
    if (!doc.isObject()) return;
    emit jsonReceived(doc.object());
}
