// ===================================================================
// [REVERT] 文件名: ticketsclient.h
// [REVERT] 内容: 回退此文件，移除了 locateLog() 函数和 logLocated(), locateLogFailed() 信号，
// [REVERT]       恢复到纯客户端日志定位之前的状态。
// ===================================================================
#pragma once

#include <QObject>
#include <QJsonArray>

class TcpClient;

class TicketsClient : public QObject {
    Q_OBJECT
public:
    explicit TicketsClient(TcpClient* tcp, QObject* parent=nullptr);
    void list(int page, int pageSize);
    void logs(int ticketId, int page, int pageSize);
    void faults(int ticketId, int page, int pageSize);
    // [DEL] 移除了向服务器发送定位请求的函数
    // void locateLog(int ticketId, const QString& faultTs);

signals:
    void listed(const QJsonArray& rows);
    void listFailed(const QString& reason);
    void logsListed(int ticketId, const QJsonArray& rows);
    void logsFailed(const QString& reason);
    void faultsListed(int ticketId, const QJsonArray& rows);
    void faultsFailed(const QString& reason);
    // [DEL] 移除了服务器定位成功/失败的信号
    // void logLocated(int page, const QString& ts);
    // void locateLogFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
};
