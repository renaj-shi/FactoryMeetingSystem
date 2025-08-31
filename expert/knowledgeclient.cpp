#include "knowledgeclient.h"
#include "tcpclient.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

KnowledgeClient::KnowledgeClient(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &KnowledgeClient::onLine);
}

void KnowledgeClient::list(int page, int pageSize) {
    if (tcp_) tcp_->sendLine(QString("KB|LIST|%1|%2").arg(page).arg(pageSize));
}

void KnowledgeClient::get(int id) {
    if (tcp_) tcp_->sendLine(QString("KB|GET|%1").arg(id));
}

void KnowledgeClient::onLine(const QString& line) {
    if (!line.startsWith("KB|")) return;

    const int p0 = line.indexOf('|');
    const int p1 = line.indexOf('|', p0+1);
    const int p2 = line.indexOf('|', p1+1);
    if (p0<0 || p1<0 || p2<0) return;

    const QString sub = line.mid(p0+1, p1-(p0+1)); // LIST/GET
    const QString st  = line.mid(p1+1, p2-(p1+1)); // OK/ERR
    const QString payload = line.mid(p2+1);

    if (sub == "LIST") {
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
    } else if (sub == "GET") {
        if (st == "OK") {
            QJsonParseError err{};
            const auto doc = QJsonDocument::fromJson(payload.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                const QJsonObject obj = doc.object();
                emit got(obj.value("id").toInt(), obj);
            } else {
                emit getFailed(QString("JSON解析失败: %1").arg(err.errorString()));
            }
        } else {
            emit getFailed(payload);
        }
    }
}
