#include "ticketsclient.h"
#include "tcpclient.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

TicketsClient::TicketsClient(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &TicketsClient::onLine);
}

void TicketsClient::list(int page, int pageSize) {
    if (tcp_) tcp_->sendLine(QString("TICKETS|LIST|%1|%2").arg(page).arg(pageSize));
}

void TicketsClient::onLine(const QString& line) {
    if (!line.startsWith("TICKETS|")) return;
    // TICKETS|LIST|OK|<json> 或 TICKETS|LIST|ERR|reason
    const int p0 = line.indexOf('|');
    const int p1 = line.indexOf('|', p0+1);
    const int p2 = line.indexOf('|', p1+1);
    if (p0<0 || p1<0 || p2<0) return;

    const QString sub = line.mid(p0+1, p1-(p0+1)); // LIST
    const QString st  = line.mid(p1+1, p2-(p1+1)); // OK/ERR
    const QString payload = line.mid(p2+1);

    if (sub != "LIST") return;

    if (st == "OK") {
        QJsonParseError err{};
        const auto doc = QJsonDocument::fromJson(payload.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isArray()) {
            emit listed(doc.array());
        } else {
            emit listFailed(QString("JSON解析失败: %1").arg(err.errorString()));
        }
    } else {
        emit listFailed(payload);
    }
}
