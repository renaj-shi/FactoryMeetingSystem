#pragma once
#include <QObject>

class QTcpSocket;

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent=nullptr);

    void connectTo(const QString& host, quint16 port);
    void close();
    void sendLine(const QString& line);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& err);
    void lineReceived(const QString& line);  // 按行抛出

private slots:
    void onReadyRead();

private:
    QTcpSocket* sock_{nullptr};
    QByteArray  buf_;
};
