#pragma once
#include <QObject>
#include <QJsonArray>

class TcpClient;

class TicketsClient : public QObject {
    Q_OBJECT
public:
    explicit TicketsClient(TcpClient* tcp, QObject* parent=nullptr);
    void list(int page=1, int pageSize=50);

signals:
    void listed(const QJsonArray& rows);
    void listFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
};
