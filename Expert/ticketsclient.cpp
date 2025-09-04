// ===================================================================
// [REVERT] 文件名: ticketsclient.cpp
// [REVERT] 内容: 回退此文件，移除了 locateLog() 的实现和 onLine() 中对定位响应的处理逻辑。
// ===================================================================
#include "ticketsclient.h"
#include "tcpclient.h"
#include <QStringList>
#include <QJsonDocument>
#include <QDebug>

TicketsClient::TicketsClient(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &TicketsClient::onLine);
}

void TicketsClient::list(int page, int pageSize) {
    tcp_->sendLine(QString("TICKETS|LIST|%1|%2").arg(page).arg(pageSize));
}

void TicketsClient::logs(int ticketId, int page, int pageSize) {
    tcp_->sendLine(QString("TICKETS|LOGS|%1|%2|%3").arg(ticketId).arg(page).arg(pageSize));
}

void TicketsClient::faults(int ticketId, int page, int pageSize) {
    tcp_->sendLine(QString("TICKETS|FAULTS|%1|%2|%3").arg(ticketId).arg(page).arg(pageSize));
}

// [DEL] 移除了 locateLog 函数的实现
// void TicketsClient::locateLog(int ticketId, const QString& faultTs) {
//     tcp_->sendLine(QString("TICKETS|LOCATE_LOG|%1|%2").arg(ticketId).arg(faultTs));
// }

void TicketsClient::onLine(const QString& line) {
    if (line.startsWith("TICKETS|")) {
        qDebug() << "[TicketsClient] Received line:" << line;
    }

    const QStringList parts = line.split('|');
    if (parts.size() < 2) return;
    const QString type = parts[0];
    const QString subtype = parts[1];

    if (type != "TICKETS") return;

    if (subtype == "LIST") {
        const int jsonIndex = (parts.size() > 3 && parts[2] == "OK") ? 3 : 2;
        if (parts.size() > jsonIndex) {
            const QJsonDocument doc = QJsonDocument::fromJson(parts[jsonIndex].toUtf8());
            if (doc.isArray()) {
                qDebug() << "[TicketsClient] Emitting 'listed' signal with array size:" << doc.array().size();
                emit listed(doc.array());
            } else {
                qDebug() << "[Tickets-Client] Error: Failed to parse JSON array from server response.";
            }
        }
    } else if (subtype == "LIST_FAILED") {
        emit listFailed(parts.size() > 2 ? parts[2] : tr("未知错误"));
    } else if (subtype == "LOGS") {
        const int jsonIndex = (parts.size() > 4 && parts[3] == "OK") ? 4 : 3;
        if (parts.size() > jsonIndex) {
            const int ticketId = parts[2].toInt();
            const QJsonDocument doc = QJsonDocument::fromJson(parts[jsonIndex].toUtf8());
            if (doc.isArray()) emit logsListed(ticketId, doc.array());
        }
    } else if (subtype == "LOGS_FAILED") {
        emit logsFailed(parts.size() > 2 ? parts[2] : tr("未知错误"));
    } else if (subtype == "FAULTS") {
        const int jsonIndex = (parts.size() > 4 && parts[3] == "OK") ? 4 : 3;
        if (parts.size() > jsonIndex) {
            const int ticketId = parts[2].toInt();
            const QJsonDocument doc = QJsonDocument::fromJson(parts[jsonIndex].toUtf8());
            if (doc.isArray()) emit faultsListed(ticketId, doc.array());
        }
    } else if (subtype == "FAULTS_FAILED") {
        emit faultsFailed(parts.size() > 2 ? parts[2] : tr("未知错误"));
    }
    // [DEL] 移除了对服务器定位响应的处理逻辑
    // else if (subtype == "LOCATE_LOG_SUCCESS") {
    //     if (parts.size() > 3) {
    //         const int page = parts[2].toInt();
    //         const QString ts = parts[3];
    //         qDebug() << "[TicketsClient] Log location received! Page:" << page << "Timestamp:" << ts << ". Emitting signal...";
    //         emit logLocated(page, ts);
    //     }
    // } else if (subtype == "LOCATE_LOG_FAILED") {
    //     emit locateLogFailed(parts.size() > 2 ? parts[2] : tr("定位失败"));
    // }
}
