// ===================================================================
// [MOD] 文件名: mainwindow.cpp
// [MOD] 内容: 1. [FIX] 将TCP连接状态的信号槽连接逻辑恢复到 bindServices() 中，
// [MOD]          使其成为连接状态的唯一控制中心。
// [MOD]       2. [FIX] 修改了这些连接中的lambda函数，使其能同时更新 WelcomeWidget 和 (登录后的) SessionWidget。
// ===================================================================
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logindialog.h"
#include "sessionwidget.h"

#include "welcomewidget.h"
#include "registerdialog.h"
#include <QLineEdit>

#include <QStatusBar>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "tcpclient.h"
#include "authmanager.h"
#include "workordermanager.h"
#include "telemetryclient.h"
#include "chatclient.h"
#include "meetingdialog.h"
#include "ticketsclient.h"
#include "ticketsviewdialog.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle(tr("工业现场远程专家支持系统·专家端"));
    setupUiEx();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiEx() {
    resize(1200, 750);

    welcome_ = new WelcomeWidget(this);
    setCentralWidget(welcome_);
    statusBar()->showMessage(tr("未登录"));

    connect(welcome_, &WelcomeWidget::loginClicked, this, &MainWindow::showLogin);
}

void MainWindow::bindServices(TcpClient* tcp,
                              AuthManager* auth,
                              WorkOrderManager* orders,
                              TelemetryClient* telem) {
    tcp_   = tcp;
    auth_  = auth;
    orders_= orders;
    telem_ = telem;

    // 登录对话框
    loginDlg_ = new LoginDialog(this);
    connect(loginDlg_, &LoginDialog::loginRequested,    auth_, &AuthManager::login);
    connect(loginDlg_, &LoginDialog::openRegisterDialog, this, [this]{
        if (!regDlg_) {
            regDlg_ = new RegisterDialog(this);
            connect(regDlg_, &RegisterDialog::registerRequested, this, [this](const QString& u, const QString& p){
                pendingRegUser_ = u;
                pendingRegPwd_  = p;
                if (auth_) auth_->registerUser(u, p);
            });
        }
        regDlg_->setStatus(tr("填写完成后点击‘注册’"));
        regDlg_->setBusy(false);
        regDlg_->show();
        regDlg_->raise();
        regDlg_->activateWindow();
    });

    connect(auth_, &AuthManager::loginSucceeded, this, &MainWindow::onLoginSucceeded);
    connect(auth_, &AuthManager::loginFailed,    this, &MainWindow::onLoginFailed);
    connect(auth_, &AuthManager::registered, this, [this]{
        if (regDlg_) {
            regDlg_->setBusy(false);
            regDlg_->setStatus(tr("注册成功"), false);
            regDlg_->accept();
            regDlg_->hide();
        }
        if (loginDlg_) {
            loginDlg_->setBusy(false);
            loginDlg_->setUserAndPwd(pendingRegUser_, pendingRegPwd_);
            loginDlg_->setStatus(tr("注册成功，请直接点击登录"), false);
            showLogin();
        }
        statusBar()->showMessage(tr("注册成功"));
    });
    connect(auth_, &AuthManager::registerFailed, this, [this](const QString& reason){
        if (regDlg_) {
            regDlg_->setBusy(false);
            regDlg_->setStatus(tr("注册失败：%1").arg(reason), true);
        }
        if (loginDlg_) {
            loginDlg_->setBusy(false);
            loginDlg_->setStatus(tr("注册失败：%1").arg(reason), true);
        }
        statusBar()->showMessage(tr("注册失败：%1").arg(reason));
    });

    // [FIX] 将TCP连接状态的信号连接恢复到此处，作为唯一的控制中心
    if (tcp_) {
        connect(tcp_, &TcpClient::connected, this, [this]{
            statusBar()->showMessage(tr("已连接专家服务器"));
            if (loginDlg_) { loginDlg_->setBusy(false); loginDlg_->setStatus(tr("已连接，请登录")); }
            // [FIX] 同时更新欢迎页面和(可能存在的)主工作区
            if (welcome_)  { welcome_->setConnectionText(tr("连接状态：已连接服务器")); }
            if (session_)  { session_->updateConnectionStatus(true, tr("在线")); }
        });
        connect(tcp_, &TcpClient::disconnected, this, [this]{
            statusBar()->showMessage(tr("与服务器断开"));
            if (loginDlg_) { loginDlg_->setBusy(true); loginDlg_->setStatus(tr("未连接服务器"), true); }
            // [FIX] 同时更新欢迎页面和(可能存在的)主工作区
            if (welcome_)  { welcome_->setConnectionText(tr("连接状态：未连接服务器")); }
            if (session_)  { session_->updateConnectionStatus(false, tr("离线")); }
        });
        connect(tcp_, &TcpClient::errorOccurred, this, [this](const QString& e){
            statusBar()->showMessage(tr("网络错误: %1").arg(e));
            if (loginDlg_) { loginDlg_->setBusy(true); loginDlg_->setStatus(tr("网络错误：%1").arg(e), true); }
            // [FIX] 同时更新欢迎页面和(可能存在的)主工作区
            if (welcome_)  { welcome_->setConnectionText(tr("连接状态：%1").arg(e)); }
            if (session_)  { session_->updateConnectionStatus(false, tr("错误")); }
        });
    }

    if (orders_) {
        connect(orders_, &WorkOrderManager::inviteReceived, this, &MainWindow::showInvite);
        connect(orders_, &WorkOrderManager::joined, this, [this](const QString& oid, const QString& title){
            statusBar()->showMessage(tr("已加入工单: %1").arg(oid));
            if (session_) session_->setOrderId(oid, title);
        });
        connect(orders_, &WorkOrderManager::left, this, [this](const QString&){
            statusBar()->showMessage(tr("已离开工单"));
            if (session_) session_->setOrderId(QString());
        });
        connect(orders_, &WorkOrderManager::joinFailed, this, [this](const QString& reason){
            statusBar()->showMessage(tr("加入失败: %1").arg(reason));
        });
    }

    tickets_ = new TicketsClient(tcp_, this);
}

void MainWindow::showLogin() {
    if (!loginDlg_ || !tcp_) return;

    if (!tcp_->isConnected()) {
        loginDlg_->setBusy(true);
        loginDlg_->setStatus(tr("正在连接服务器..."));

        const QString serverIp   = QStringLiteral("127.0.0.1");
        const quint16 serverPort = 12346;
        tcp_->connectTo(serverIp, serverPort);
    }

    loginDlg_->show();
    loginDlg_->raise();
    loginDlg_->activateWindow();
}

void MainWindow::ensureSessionReady() {
    if (session_) return;
    session_ = new SessionWidget(this);
    setCentralWidget(session_);
    session_->bindServices(tcp_, auth_, orders_, telem_, tickets_);
    if (chat_) session_->attachChat(chat_);

    connect(session_, &SessionWidget::createMeetingRequested, this, &MainWindow::createMeeting);
    connect(session_, &SessionWidget::joinMeetingRequested, this, &MainWindow::joinMeeting);
}

void MainWindow::attachChat(ChatClient* chat) {
    chat_ = chat;
    if (session_) session_->attachChat(chat_);
}

void MainWindow::showInvite(const QString& orderId, const QString& title) {
    const auto ret = QMessageBox::question(this, tr("工单邀请"),
                                           tr("收到工单邀请：%1\n工单号：%2\n是否加入？").arg(title, orderId),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret == QMessageBox::Yes && orders_) {
        orders_->join(orderId);
    }
}

void MainWindow::onLoginSucceeded(const QString& userId, const QString& token) {
    Q_UNUSED(userId); Q_UNUSED(token);
    if (loginDlg_) {
        loginDlg_->setBusy(false);
        loginDlg_->accept();
        loginDlg_->hide();
    }
    ensureSessionReady();
    statusBar()->showMessage(tr("登录成功"));
}

void MainWindow::onLoginFailed(const QString& reason) {
    if (loginDlg_) {
        loginDlg_->setBusy(false);
        loginDlg_->setStatus(tr("登录失败：%1").arg(reason), true);
    } else {
        QMessageBox::warning(this, tr("登录失败"), reason);
    }
}

void MainWindow::joinMeeting(){
    MeetingDialog *dlg = new MeetingDialog("expert_user", "localhost", 12347, "", this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->show();
}

void MainWindow::createMeeting() {
    QMessageBox::information(this, "提示", "创建会议功能待实现，暂时跳转至加入会议。");
    joinMeeting();
}
