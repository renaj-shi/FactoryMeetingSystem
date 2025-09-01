#include "tcpclient.h"
#include <QTcpSocket>

TcpClient::TcpClient(QObject* parent) : QObject(parent),
    sock_(new QTcpSocket(this)) {
    connect(sock_, &QTcpSocket::connected,    this, &TcpClient::connected);
    connect(sock_, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(sock_, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, [this](auto){ emit errorOccurred(sock_->errorString()); });
#else
    connect(sock_, &QTcpSocket::errorOccurred,
            this, [this](auto){ emit errorOccurred(sock_->errorString()); });
#endif
    connect(sock_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
}

void TcpClient::connectTo(const QString& host, quint16 port) {
    sock_->connectToHost(host, port);
}

void TcpClient::close() {
    sock_->disconnectFromHost();
}

void TcpClient::sendLine(const QString& line) {
    if (!sock_ || sock_->state()!=QTcpSocket::ConnectedState) return;
    sock_->write(line.toUtf8());
    sock_->write("\n");
    sock_->flush();
}

bool TcpClient::isConnected() const {
    return sock_ && sock_->state()==QTcpSocket::ConnectedState;
}

void TcpClient::onReadyRead() {
    buf_.append(sock_->readAll());
    while (true) {
        int pos = buf_.indexOf('\n');
        if (pos < 0) break;
        QByteArray one = buf_.left(pos);
        buf_.remove(0, pos+1);
        emit lineReceived(QString::fromUtf8(one).trimmed());
    }
}
