#pragma once
#include <QObject>
#include <QTcpSocket>

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent=nullptr);
    void connectTo(const QString& host, quint16 port);
    void close();
    void sendLine(const QString& line);
    bool isConnected() const { return sock_.state() == QAbstractSocket::ConnectedState; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& err);
    void lineReceived(const QString& line);

private slots:
    void onReadyRead();

private:
    QTcpSocket sock_;
};
