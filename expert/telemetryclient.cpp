#include "telemetryclient.h"
#include "tcpclient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

TelemetryClient::TelemetryClient(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &TelemetryClient::onLine);
}

void TelemetryClient::subscribe(const QString& orderId) {
    orderId_ = orderId;
    if (tcp_) tcp_->sendLine(QString("TELE|SUB|%1").arg(orderId));
}

void TelemetryClient::unsubscribe() {
    if (!orderId_.isEmpty() && tcp_) {
        tcp_->sendLine(QString("TELE|UNSUB|%1").arg(orderId_));
    }
    orderId_.clear();
}

void TelemetryClient::onLine(const QString& line) {
    if (!line.startsWith("TELE|")) return;

    int p0 = line.indexOf('|');
    int p1 = line.indexOf('|', p0+1);
    int p2 = line.indexOf('|', p1+1);
    if (p0<0 || p1<0 || p2<0) return;

    const QString sub = line.mid(p0+1, p1-(p0+1));    // DATA
    const QString oid = line.mid(p1+1, p2-(p1+1));    // orderId
    const QString payload = line.mid(p2+1);           // JSON

    if (sub != "DATA") return;

    QJsonParseError perr{};
    const auto doc = QJsonDocument::fromJson(payload.toUtf8(), &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "TELE payload parse error:" << perr.errorString() << payload;
        return;
    }
    const QJsonObject obj = doc.object();
    emit metricsUpdated(oid, obj);

    // logs
    if (obj.contains("logs") && obj.value("logs").isArray()) {
        const QJsonArray logs = obj.value("logs").toArray();
        for (const QJsonValue& v : logs) {
            emit logAppended(oid, v.toString());
        }
    }
    // faults
    if (obj.contains("faults") && obj.value("faults").isArray()) {
        const QJsonArray faults = obj.value("faults").toArray();
        for (const QJsonValue& v : faults) {
            const auto o = v.toObject();
            emit faultRaised(oid,
                             o.value("code").toString(),
                             o.value("text").toString(),
                             o.value("level").toString());
        }
    }
}
