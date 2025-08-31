#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject* parent) : QObject(parent) {
    connect(&sock_, &QTcpSocket::connected,    this, &TcpClient::connected);
    connect(&sock_, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(&sock_, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, [this](auto){ emit errorOccurred(sock_.errorString()); });
#else
    connect(&sock_, &QTcpSocket::errorOccurred,
            this, [this](auto){ emit errorOccurred(sock_.errorString()); });
#endif
    connect(&sock_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
}

void TcpClient::connectTo(const QString& host, quint16 port) {
    sock_.connectToHost(host, port);
}

void TcpClient::close() { sock_.disconnectFromHost(); }

void TcpClient::sendLine(const QString& line) {
    if (sock_.state() == QAbstractSocket::ConnectedState) {
        QByteArray data = line.toUtf8();
        data.append('\n');
        sock_.write(data);
        sock_.flush();
    }
}

void TcpClient::onReadyRead() {
    while (sock_.canReadLine()) {
        const QByteArray line = sock_.readLine();
        emit lineReceived(QString::fromUtf8(line).trimmed());
    }
}
