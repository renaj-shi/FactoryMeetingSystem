// ===================================================================
// [MOD] 文件名: tcpclient.cpp
// [MOD] 内容: 修正了构造函数中 connect 语句的语法，以消除 "No such slot" 警告。
// ===================================================================
#include "tcpclient.h"
#include <QTcpSocket>
#include <QHostAddress>

TcpClient::TcpClient(QObject* parent) : QObject(parent) {
    sock_ = new QTcpSocket(this);
    connect(sock_, &QTcpSocket::connected,    this, &TcpClient::onConnected);
    connect(sock_, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(sock_, &QTcpSocket::readyRead,    this, &TcpClient::onReadyRead);
    // [FIX] 使用了新的、非弃用的错误信号/槽连接方式的正确语法
    connect(sock_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void TcpClient::connectTo(const QString& host, quint16 port) {
    if (sock_->state() != QAbstractSocket::UnconnectedState) {
        return;
    }
    sock_->connectToHost(host, port);
}

void TcpClient::disconnectFromHost() {
    sock_->disconnectFromHost();
}

void TcpClient::sendLine(const QString& line) {
    if (!isConnected()) return;
    sock_->write((line + "\n").toUtf8());
}

bool TcpClient::isConnected() const {
    return sock_ && sock_->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onConnected() {
    emit connected();
}

void TcpClient::onDisconnected() {
    emit disconnected();
}

void TcpClient::onReadyRead() {
    buffer_.append(sock_->readAll());
    while (true) {
        const int pos = buffer_.indexOf('\n');
        if (pos < 0) break;
        QByteArray lineBytes = buffer_.left(pos);
        buffer_.remove(0, pos + 1);
        emit lineReceived(QString::fromUtf8(lineBytes));
    }
}

// [MOD] 将参数类型从 int 改为 QAbstractSocket::SocketError 以匹配信号
void TcpClient::onError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit errorOccurred(sock_->errorString());
}
