#include "authmanager.h"
#include "tcpclient.h"
#include <QStringList>

AuthManager::AuthManager(TcpClient* tcp, QObject* parent)
    : QObject(parent), tcp_(tcp) {
    connect(tcp_, &TcpClient::lineReceived, this, &AuthManager::onLine);
}

void AuthManager::login(const QString& user, const QString& pwdPlain) {
    user_ = user;
    // 注意：后端当前按明文比对密码，这里不要做 sha256，否则会匹配失败
    tcp_->sendLine(QString("LOGIN|%1|%2").arg(user, pwdPlain));
}

void AuthManager::registerUser(const QString& user, const QString& pwdPlain) {
    tcp_->sendLine(QString("REGISTER|%1|%2").arg(user, pwdPlain));
}

void AuthManager::onLine(const QString& line) {
    const QStringList parts = line.split('|');
    if (parts.size() < 2) return;
    const QString type = parts[0];
    const QString status = parts[1];

    if (type == "LOGIN") {
        if (status == "SUCCESS") {
            emit loginSucceeded(user_, QString()); // 目前没有 token
        } else {
            const QString reason = parts.size() > 2 ? parts[2] : tr("未知错误");
            emit loginFailed(reason);
        }
    } else if (type == "REGISTER") {
        if (status == "SUCCESS") {
            emit registered();
        } else {
            const QString reason = parts.size() > 2 ? parts[2] : tr("未知错误");
            emit registerFailed(reason);
        }
    }
    // TYPE|EXPERT|ACK 等信息可忽略或在 MainWindow 状态栏显示
}
