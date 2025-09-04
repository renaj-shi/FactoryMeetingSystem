#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("登录 / 注册"));
    setModal(true);
    resize(380, 230);

    // [ADD] 统一浅色样式（不使用图片）
    setStyleSheet(
        "QDialog{background:#fafafa;}"
        "QLineEdit{border:1px solid #cfd8dc;border-radius:4px;padding:6px;}"
        "QLineEdit:focus{border:1px solid #2f80ed;}"
        "QPushButton{padding:6px 12px;}"
        "QPushButton:enabled:hover{background:#eaf2fe;}"
        "QLabel.hint{color:#666;}"
        "QLabel.error{color:#d33;}"
        );

    auto* ly = new QVBoxLayout(this);
    ly->setContentsMargins(18, 16, 18, 16);
    ly->setSpacing(10);

    // [ADD] 标题与提示
    auto* title = new QLabel(tr("欢迎使用专家端"), this);
    QFont tf = title->font(); tf.setPointSize(tf.pointSize()+3); tf.setBold(true);
    title->setFont(tf);
    ly->addWidget(title);

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

    // [ADD] 显示密码选项
    auto* optRow = new QHBoxLayout();
    optRow->addStretch();
    showPwd_ = new QCheckBox(tr("显示密码"), this);
    connect(showPwd_, &QCheckBox::toggled, this, &LoginDialog::onToggleShowPwd);
    optRow->addWidget(showPwd_);
    ly->addLayout(optRow);

    statusLabel_ = new QLabel(this);
    statusLabel_->setProperty("class", "hint");
    statusLabel_->setText(tr("请输入用户名和密码进行登录"));
    ly->addWidget(statusLabel_);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    loginBtn_    = new QPushButton(tr("登录"), this);
    // [MOD] 文案微调：强调会打开注册页面
    registerBtn_ = new QPushButton(tr("注册..."), this); // [MOD]
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

// 在 logindialog.cpp 中
void LoginDialog::setStatus(const QString& text, bool isError) {
    statusLabel_->setText(text);
    // 恢复为正确的代码：直接设置样式表即可，不需要 polish/unpolish
    statusLabel_->setStyleSheet(isError ? "color:#d33;" : "color:#888;");
}

void LoginDialog::setUserAndPwd(const QString& user, const QString& pwd) {
    // [ADD] 注册成功后回填账号信息
    userEdit_->setText(user);
    pwdEdit_->setText(pwd);
}

void LoginDialog::onLoginClicked() {
    const QString u = userEdit_->text().trimmed();
    const QString p = pwdEdit_->text();
    if (u.isEmpty() || p.isEmpty()) { setStatus(tr("请输入用户名和密码"), true); return; }
    setBusy(true); setStatus(tr("正在登录..."));
    emit loginRequested(u, p);
}

void LoginDialog::onRegisterClicked() {
    // [MOD] 改为打开独立注册页面，不再在此页直接注册
    emit openRegisterDialog(); // [MOD]
}

void LoginDialog::onToggleShowPwd(bool on) {
    // [ADD] 显示/隐藏密码
    pwdEdit_->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}
