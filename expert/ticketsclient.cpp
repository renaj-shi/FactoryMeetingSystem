#include "ticketsclient.h"
#include "tcpclient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>

TicketsClient::TicketsClient(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &TicketsClient::onLine);
}

void TicketsClient::list(int page, int pageSize) {
    if (tcp_) tcp_->sendLine(QString("TICKETS|LIST|%1|%2").arg(page).arg(pageSize));
}

void TicketsClient::logs(int ticketId, int page, int pageSize) {
    pendingLogsTid_ = ticketId;
    if (tcp_) tcp_->sendLine(QString("TICKETS|LOGS|%1|%2|%3").arg(ticketId).arg(page).arg(pageSize));
}

void TicketsClient::faults(int ticketId, int page, int pageSize) {
    pendingFaultsTid_ = ticketId;
    if (tcp_) tcp_->sendLine(QString("TICKETS|FAULTS|%1|%2|%3").arg(ticketId).arg(page).arg(pageSize));
}

void TicketsClient::onLine(const QString& line) {
    if (!line.startsWith("TICKETS|")) return;

    // TICKETS|SUB|OK|payload 或 TICKETS|SUB|ERR|reason
    int p0 = line.indexOf('|');
    int p1 = line.indexOf('|', p0+1);
    int p2 = line.indexOf('|', p1+1);
    if (p0<0 || p1<0 || p2<0) return;

    const QString sub = line.mid(p0+1, p1-(p0+1)); // LIST/LOGS/FAULTS
    const QString st  = line.mid(p1+1, p2-(p1+1)); // OK/ERR
    const QString payload = line.mid(p2+1);

    auto parseArray = [&](const QString& s, QJsonArray& out, QString& errStr)->bool{
        QJsonParseError err{}; auto doc = QJsonDocument::fromJson(s.toUtf8(), &err);
        if (err.error==QJsonParseError::NoError && doc.isArray()) { out = doc.array(); return true; }
        errStr = err.errorString(); return false;
    };

    if (sub == "LIST") {
        if (st == "OK") {
            QJsonArray arr; QString e;
            if (parseArray(payload, arr, e)) emit listed(arr);
            else emit listFailed(QString("JSON解析失败: %1").arg(e));
        } else {
            emit listFailed(payload);
        }
    } else if (sub == "LOGS") {
        if (st == "OK") {
            QJsonArray arr; QString e;
            if (parseArray(payload, arr, e)) emit logsListed(pendingLogsTid_, arr);
            else emit logsFailed(QString("JSON解析失败: %1").arg(e));
        } else {
            emit logsFailed(payload);
        }
    } else if (sub == "FAULTS") {
        if (st == "OK") {
            QJsonArray arr; QString e;
            if (parseArray(payload, arr, e)) emit faultsListed(pendingFaultsTid_, arr);
            else emit faultsFailed(QString("JSON解析失败: %1").arg(e));
        } else {
            emit faultsFailed(payload);
        }
    }
}
