// ===================================================================
// [FIX] 文件名: tcpclient.h
// [FIX] 内容: 将 onError 槽函数的参数类型从 int 修改为 QAbstractSocket::SocketError，
// [FIX] 以匹配其在 .cpp 文件中的实现和 Qt5 的信号机制。
// ===================================================================
#pragma once

#include <QObject>
#include <QString>
#include <QAbstractSocket> // [ADD] 需要包含此头文件以使用 QAbstractSocket::SocketError

class QTcpSocket;

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent=nullptr);
    void connectTo(const QString& host, quint16 port);
    void disconnectFromHost();
    void sendLine(const QString& line);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void lineReceived(const QString& line);
    void errorOccurred(const QString& errorText);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    // [MOD] 将参数类型从 int 修改为 QAbstractSocket::SocketError
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* sock_{nullptr};
    QByteArray  buffer_;
};
