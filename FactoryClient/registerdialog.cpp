#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>
#include <QDebug>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , isPasswordVisible(false)
    , isConfirmPasswordVisible(false)
{
    ui->setupUi(this);
    setWindowTitle("用户注册");

    // 设置密码输入框为隐藏模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // 连接密码可见性切换按钮的信号和槽
    connect(ui->togglePasswordButton, &QPushButton::clicked, this, &RegisterDialog::onTogglePasswordVisibility);
    connect(ui->toggleConfirmPasswordButton, &QPushButton::clicked, this, &RegisterDialog::onToggleConfirmPasswordVisibility);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_registerConfirmButton_clicked()
{
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();

    // 验证输入
    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写所有必填字段");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "密码不匹配", "两次输入的密码不一致，请重新输入");
        return;
    }

    // 验证通过，发送注册信号
    emit registerResult(true, QString("REGISTER|%1|%2").arg(username).arg(password));
}

void RegisterDialog::on_backToLoginButton_clicked()
{
    // 发送返回登录信号
    emit backToLogin();
    this->hide();
}

void RegisterDialog::onTogglePasswordVisibility()
{
    if (isPasswordVisible) {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->togglePasswordButton->setText("👁️");
    } else {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->togglePasswordButton->setText("👁️‍🗨️");
    }
    isPasswordVisible = !isPasswordVisible;
}

void RegisterDialog::onToggleConfirmPasswordVisibility()
{
    if (isConfirmPasswordVisible) {
        ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);
        ui->toggleConfirmPasswordButton->setText("👁️");
    } else {
        ui->confirmPasswordEdit->setEchoMode(QLineEdit::Normal);
        ui->toggleConfirmPasswordButton->setText("👁️‍🗨️");
    }
    isConfirmPasswordVisible = !isConfirmPasswordVisible;
}

void RegisterDialog::onRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "注册成功", message);
    emit backToLogin();  // 发送返回登录信号
    this->hide();
}

void RegisterDialog::onRegisterError(const QString &message)
{
    QMessageBox::warning(this, "注册失败", message);
}