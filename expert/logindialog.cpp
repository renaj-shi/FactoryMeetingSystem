#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("登录 / 注册"));
    setModal(true);
    resize(380, 200);

    auto* ly = new QVBoxLayout(this);

    auto* userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel(tr("用户名:"), this));
    userEdit_ = new QLineEdit(this);
    userEdit_->setPlaceholderText(tr("请输入用户名"));
    userRow->addWidget(userEdit_);
    ly->addLayout(userRow);

    auto* pwdRow = new QHBoxLayout();
    pwdRow->addWidget(new QLabel(tr("密码:"), this));
    pwdEdit_ = new QLineEdit(this);
    pwdEdit_->setEchoMode(QLineEdit::Password);
    pwdEdit_->setPlaceholderText(tr("请输入密码"));
    pwdRow->addWidget(pwdEdit_);
    ly->addLayout(pwdRow);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("color:#888;");
    ly->addWidget(statusLabel_);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    loginBtn_    = new QPushButton(tr("登录"), this);
    registerBtn_ = new QPushButton(tr("注册"), this);
    cancelBtn_   = new QPushButton(tr("取消"), this);
    btnRow->addWidget(loginBtn_);
    btnRow->addWidget(registerBtn_);
    btnRow->addWidget(cancelBtn_);
    ly->addLayout(btnRow);

    connect(loginBtn_,    &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerBtn_, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(cancelBtn_,   &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::setBusy(bool busy) {
    userEdit_->setEnabled(!busy);
    pwdEdit_->setEnabled(!busy);
    loginBtn_->setEnabled(!busy);
    registerBtn_->setEnabled(!busy);
}

void LoginDialog::setStatus(const QString& text, bool isError) {
    statusLabel_->setText(text);
    statusLabel_->setStyleSheet(isError ? "color:#d33;" : "color:#888;");
}

void LoginDialog::onLoginClicked() {
    const QString u = userEdit_->text().trimmed();
    const QString p = pwdEdit_->text();
    if (u.isEmpty() || p.isEmpty()) { setStatus(tr("请输入用户名和密码"), true); return; }
    setBusy(true); setStatus(tr("正在登录..."));
    emit loginRequested(u, p);
}

void LoginDialog::onRegisterClicked() {
    const QString u = userEdit_->text().trimmed();
    const QString p = pwdEdit_->text();
    if (u.isEmpty() || p.isEmpty()) { setStatus(tr("请输入用户名和密码"), true); return; }
    setBusy(true); setStatus(tr("正在注册..."));
    emit registerRequested(u, p);
}
