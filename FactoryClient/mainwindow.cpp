#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "maininterfacedialog.h"
#include <QTcpSocket>
#include <QMessageBox>
#include <QDebug>
#include <QApplication> // 【增】为 qApp->quit() 包含头文件

// 构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tcpSocket(new QTcpSocket(this))
    , isConnected(false)
{
    ui->setupUi(this);
    setWindowTitle("工厂客户端");

    // 设置默认值
    ui->ipEdit->setText("127.0.0.1");
    ui->portEdit->setText("12345");

    setupConnection();

    // 初始状态禁用按钮
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);
    
    // 添加工具提示，解释按钮禁用原因
    ui->registerButton->setToolTip("请先连接到服务器");
    ui->loginButton->setToolTip("请先连接到服务器");

    // 在状态栏显示初始信息
    statusBar()->showMessage("未连接");

    // 连接密码可见性切换按钮的信号和槽
    connect(ui->togglePasswordButton, &QPushButton::clicked, this, [=]() {
        if (ui->passwordEdit->echoMode() == QLineEdit::Password) {
            ui->passwordEdit->setEchoMode(QLineEdit::Normal);
            ui->togglePasswordButton->setText("👁️‍🗨️");
        } else {
            ui->passwordEdit->setEchoMode(QLineEdit::Password);
            ui->togglePasswordButton->setText("👁️");
        }
    });

    // 初始化注册对话框
    registerDialog = new RegisterDialog(this);
    connect(registerDialog, &RegisterDialog::registerResult, this, &MainWindow::handleRegisterResult);
    connect(registerDialog, &RegisterDialog::backToLogin, this, [=]() {
        this->show();
    });
}

MainWindow::~MainWindow()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
    delete ui;
    delete registerDialog;
}

void MainWindow::setupConnection()
{
    connect(tcpSocket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &MainWindow::onError);
}

void MainWindow::on_connectButton_clicked()
{
    if (isConnected) {
        tcpSocket->disconnectFromHost();
        statusBar()->showMessage("正在断开连接...");
        return;
    }

    QString ip = ui->ipEdit->text();
    int port = ui->portEdit->text().toInt();

    if (ip.isEmpty() || port == 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的IP地址和端口号");
        return;
    }

    tcpSocket->connectToHost(ip, port);
    statusBar()->showMessage("正在连接服务器...");
    ui->connectButton->setEnabled(false);
}

void MainWindow::onConnected()
{
    isConnected = true;
    statusBar()->showMessage("已连接到工厂服务器");
    ui->connectButton->setText("断开连接");
    ui->connectButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->loginButton->setEnabled(true);

    QMessageBox::information(this, "连接成功", "成功连接到工厂服务器！");
}

void MainWindow::onDisconnected()
{
    isConnected = false;
    statusBar()->showMessage("已断开连接");
    ui->connectButton->setText("连接");
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);

    QMessageBox::information(this, "断开连接", "与服务器的连接已断开");
}

void MainWindow::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    statusBar()->showMessage("连接错误: " + tcpSocket->errorString());
    isConnected = false;
    ui->connectButton->setText("连接");
    ui->connectButton->setEnabled(true);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);

    QMessageBox::critical(this, "连接错误", "连接失败: " + tcpSocket->errorString());
}

void MainWindow::onReadyRead()
{
    while (tcpSocket->canReadLine()) {
        QString response = QString::fromUtf8(tcpSocket->readLine()).trimmed();
        qDebug() << "服务器响应:" << response;

        QStringList parts = response.split("|");
        if (parts.size() < 2) {
            continue;
        }

        QString type = parts[0];
        QString status = parts[1];

        if (type == "TYPE") {
            statusBar()->showMessage("已确认工厂客户端身份");
        }
        else if (type == "REGISTER") {
            if (status == "SUCCESS") {
                statusBar()->showMessage("注册成功");
                QString message = parts.size() > 2 ? parts[2] : "用户注册成功！";
                QMessageBox::information(this, "注册成功", message);
                if (registerDialog && registerDialog->isVisible()) {
                    registerDialog->onRegisterSuccess(message);
                }
            } else {
                QString errorMsg = parts.size() > 2 ? parts[2] : "未知错误";
                statusBar()->showMessage("注册失败: " + errorMsg);
                QMessageBox::warning(this, "注册失败", errorMsg);
                if (registerDialog && registerDialog->isVisible()) {
                    registerDialog->onRegisterError(errorMsg);
                }
            }
        }
        else if (type == "LOGIN") {
            qDebug() << "[Login] 接收到LOGIN响应，status=" << status;
            if (status == "SUCCESS") {
                statusBar()->showMessage("登录成功");
                qDebug() << "[Login] 登录成功，准备创建主界面，m_username=" << m_username;
                
                // 断开socket的readyRead信号连接，避免登录窗口继续读取数据
                disconnect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
                qDebug() << "[Login] 已断开登录窗口的readyRead信号连接";

                // 创建主界面
                MainInterfaceDialog *mainInterface = new MainInterfaceDialog(nullptr);
                qDebug() << "[Login] 已创建MainInterfaceDialog实例";
                
                // 设置主界面的属性，确保关闭时自动销毁
                mainInterface->setAttribute(Qt::WA_DeleteOnClose);
                qDebug() << "[Login] 已设置WA_DeleteOnClose属性";
                
                // 将socket的父对象设置为主界面，保证登录窗口关闭后socket仍在
                tcpSocket->setParent(mainInterface);
                qDebug() << "[Login] 已将socket的父对象设置为主界面";
                
                // 将socket注入到主界面
                qDebug() << "[Login] 准备调用setConnection方法";
                mainInterface->setConnection(tcpSocket, m_username);
                qDebug() << "[Login] 已调用setConnection方法";

                // 清空当前窗口的socket指针
                this->hide();
                tcpSocket = nullptr;
                qDebug() << "[Login] 已隐藏登录窗口，tcpSocket置为nullptr";

                // 使用 exec() 模态显示，并处理后续逻辑
                mainInterface->exec();
                qDebug() << "[Login] 主界面已关闭";

                // 退出整个应用程序
                qApp->quit();
                
                return; // 确保执行完退出逻辑后函数返回

            } else {
                QString errorMsg = parts.size() > 2 ? parts[2] : "未知错误";
                statusBar()->showMessage("登录失败: " + errorMsg);
                QMessageBox::warning(this, "登录失败", errorMsg);
            }
        }
        else if (type == "ERROR") {
            QString errorMsg = parts.size() > 1 ? parts[1] : "未知错误";
            statusBar()->showMessage("错误: " + errorMsg);
            QMessageBox::warning(this, "错误", errorMsg);
        }
    }
}

void MainWindow::on_registerButton_clicked()
{
    if (!isConnected) {
        QMessageBox::warning(this, "错误", "请先连接到服务器");
        return;
    }

    showRegisterDialog();
}

void MainWindow::on_loginButton_clicked()
{
    if (!isConnected) {
        QMessageBox::warning(this, "错误", "请先连接到服务器");
        return;
    }

    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    // 【增】发送前记录用户名
    m_username = username;

    QString message = QString("LOGIN|%1|%2").arg(username).arg(password);
    tcpSocket->write(message.toUtf8() + "\n");
    statusBar()->showMessage("正在发送登录请求...");
}

void MainWindow::handleRegisterResult(bool success, const QString &message)
{
    if (success) {
        tcpSocket->write(message.toUtf8() + "\n");
        statusBar()->showMessage("正在发送注册请求...");
    }
}

void MainWindow::showRegisterDialog()
{
    this->hide();
    registerDialog->show();
}