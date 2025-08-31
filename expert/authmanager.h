#pragma once
#include <QObject>
#include <QString>

class TcpClient;

class AuthManager : public QObject {
    Q_OBJECT
public:
    explicit AuthManager(TcpClient* tcp, QObject* parent=nullptr);
    void login(const QString& user, const QString& pwdPlain);       // 发送 LOGIN|user|pwd
    void registerUser(const QString& user, const QString& pwdPlain); // 发送 REGISTER|user|pwd

    QString userId() const { return user_; } // 服务端未返回 token，这里先用用户名代替
    QString token()  const { return QString(); }

signals:
    void loginSucceeded(const QString& userId, const QString& token);
    void loginFailed(const QString& reason);
    void registered();
    void registerFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
    QString user_;
};
