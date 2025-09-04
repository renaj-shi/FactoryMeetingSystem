#include "registerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

RegisterDialog::RegisterDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("注册新账户"));
    setModal(true);
    resize(420, 260);

    // [ADD] 统一浅色样式（不使用图片）
    setStyleSheet(
        "QDialog{background:#fafafa;}"
        "QLineEdit{border:1px solid #cfd8dc;border-radius:4px;padding:6px;}"
        "QLineEdit:focus{border:1px solid #2f80ed;}"
        "QPushButton{padding:6px 14px;}"
        "QPushButton:enabled:hover{background:#eaf2fe;}"
        "QLabel.hint{color:#666;}"
        "QLabel.error{color:#d33;}"
        "QLabel.ok{color:#2e7d32;}"
        );

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(12);

    auto* title = new QLabel(tr("创建专家账号"), this);
    QFont f = title->font(); f.setPointSize(f.pointSize()+4); f.setBold(true);
    title->setFont(f);
    root->addWidget(title);

    auto* userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel(tr("用户名:"), this));
    userEdit_ = new QLineEdit(this);
    userEdit_->setPlaceholderText(tr("请输入用户名"));
    userRow->addWidget(userEdit_);
    root->addLayout(userRow);

    auto* pwdRow = new QHBoxLayout();
    pwdRow->addWidget(new QLabel(tr("密码:"), this));
    pwdEdit_ = new QLineEdit(this);
    pwdEdit_->setEchoMode(QLineEdit::Password);
    pwdEdit_->setPlaceholderText(tr("建议包含字母与数字"));
    pwdRow->addWidget(pwdEdit_);
    root->addLayout(pwdRow);

    auto* confirmRow = new QHBoxLayout();
    confirmRow->addWidget(new QLabel(tr("确认密码:"), this));
    confirmEdit_ = new QLineEdit(this);
    confirmEdit_->setEchoMode(QLineEdit::Password);
    confirmEdit_->setPlaceholderText(tr("请再次输入密码"));
    confirmRow->addWidget(confirmEdit_);
    root->addLayout(confirmRow);

    auto* optRow = new QHBoxLayout();
    showPwd_ = new QCheckBox(tr("显示密码"), this);
    connect(showPwd_, &QCheckBox::toggled, this, &RegisterDialog::onToggleShowPwd);
    optRow->addStretch();
    optRow->addWidget(showPwd_);
    root->addLayout(optRow);

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName("status");
    statusLabel_->setProperty("class", "hint");
    statusLabel_->setText(tr("填写完成后点击‘注册’"));
    root->addWidget(statusLabel_);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    okBtn_ = new QPushButton(tr("注册"), this);
    cancelBtn_ = new QPushButton(tr("取消"), this);
    btnRow->addWidget(okBtn_);
    btnRow->addWidget(cancelBtn_);
    root->addLayout(btnRow);

    connect(okBtn_, &QPushButton::clicked, this, &RegisterDialog::onSubmit);
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::setBusy(bool busy) {
    userEdit_->setEnabled(!busy);
    pwdEdit_->setEnabled(!busy);
    confirmEdit_->setEnabled(!busy);
    showPwd_->setEnabled(!busy);
    okBtn_->setEnabled(!busy);
}

// 在 registerdialog.cpp 中
void RegisterDialog::setStatus(const QString& text, bool isError) {
    statusLabel_->setText(text);
    // 恢复为正确的代码：直接设置样式表即可，不需要 polish/unpolish
    statusLabel_->setStyleSheet(isError ? "color:#d33;" : "color:#888;");
}

void RegisterDialog::presetUsername(const QString& user) {
    if (!user.isEmpty()) userEdit_->setText(user);
}

void RegisterDialog::onToggleShowPwd(bool on) {
    pwdEdit_->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    confirmEdit_->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

void RegisterDialog::onSubmit() {
    const QString u = userEdit_->text().trimmed();
    const QString p = pwdEdit_->text();
    const QString c = confirmEdit_->text();

    if (p != c) {
        setStatus(tr("两次输入的密码不一致"), true);
        return;
    }

    setBusy(true);
    setStatus(tr("正在注册..."));
    // [ADD] 发出注册请求，由外部连接到 AuthManager::registerUser
    emit registerRequested(u, p);
}
