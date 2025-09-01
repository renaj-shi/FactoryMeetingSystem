#pragma once
#include <QObject>
#include <QJsonArray>

class TcpClient;

class TicketsClient : public QObject {
    Q_OBJECT
public:
    explicit TicketsClient(TcpClient* tcp, QObject* parent=nullptr);

    // 历史工单列表
    void list(int page=1, int pageSize=50);
    // 指定工单的设备日志/故障
    void logs(int ticketId, int page=1, int pageSize=100);
    void faults(int ticketId, int page=1, int pageSize=100);

signals:
    void listed(const QJsonArray& rows);
    void listFailed(const QString& reason);

    void logsListed(int ticketId, const QJsonArray& rows);
    void logsFailed(const QString& reason);

    void faultsListed(int ticketId, const QJsonArray& rows);
    void faultsFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
    int pendingLogsTid_{-1};
    int pendingFaultsTid_{-1};
};
