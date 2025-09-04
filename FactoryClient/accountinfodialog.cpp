#include "accountinfodialog.h"
#include "ui_accountinfodialog.h"
#include <QMessageBox>

AccountInfoDialog::AccountInfoDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountInfoDialog)
{
    ui->setupUi(this);

    // 设置无边框样式
    this->setWindowFlags(Qt::Widget);
    
    // 连接按钮信号槽
    connect(ui->logoutButton, &QPushButton::clicked, this, &AccountInfoDialog::on_logoutButton_clicked);
    connect(ui->profileButton, &QPushButton::clicked, this, &AccountInfoDialog::on_profileButton_clicked);
    connect(ui->passwordButton, &QPushButton::clicked, this, &AccountInfoDialog::on_passwordButton_clicked);
}

AccountInfoDialog::~AccountInfoDialog()
{
    delete ui;
}

void AccountInfoDialog::setUsername(const QString &username)
{
    ui->usernameLabel->setText(username);
    
    // 如果用户名不为空，设置头像文字为用户名的前两个字符
    if (!username.isEmpty()) {
        QString avatarText = username.left(2).toUpper();
        ui->avatarLabel->setText(avatarText);
    }
}

void AccountInfoDialog::on_logoutButton_clicked()
{
    emit logoutRequested();
}

void AccountInfoDialog::on_profileButton_clicked()
{
    emit profileEditRequested();
    
    // 弹出窗口显示用户名
    QString username = ui->usernameLabel->text();
    QMessageBox::information(this, "个人资料", "用户名: " + username);
}

void AccountInfoDialog::on_passwordButton_clicked()
{
    emit passwordChangeRequested();
}