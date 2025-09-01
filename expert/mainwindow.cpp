#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logindialog.h"
#include "sessionwidget.h"

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
// 仅保留历史工单
#include "ticketsclient.h"
#include "ticketsviewdialog.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setupUiEx();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiEx() {
    resize(1000, 650);
    // 启动时仅放空白中央区，登录成功后再创建 SessionWidget
    setCentralWidget(new QWidget(this));
    statusBar()->showMessage(tr("未登录"));
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
    connect(loginDlg_, &LoginDialog::registerRequested, auth_, &AuthManager::registerUser);

    connect(auth_, &AuthManager::loginSucceeded, this, &MainWindow::onLoginSucceeded);
    connect(auth_, &AuthManager::loginFailed,    this, &MainWindow::onLoginFailed);

    connect(auth_, &AuthManager::registered, this, [this]{
        if (loginDlg_) { loginDlg_->setBusy(false); loginDlg_->setStatus(tr("注册成功，请登录")); }
        statusBar()->showMessage(tr("注册成功"));
    });
    connect(auth_, &AuthManager::registerFailed, this, [this](const QString& reason){
        if (loginDlg_) { loginDlg_->setBusy(false); loginDlg_->setStatus(tr("注册失败：%1").arg(reason), true); }
        statusBar()->showMessage(tr("注册失败：%1").arg(reason));
    });

    // TCP 连接状态
    if (tcp_) {
        connect(tcp_, &TcpClient::connected, this, [this]{
            statusBar()->showMessage(tr("已连接专家服务器"));
            if (loginDlg_) { loginDlg_->setBusy(false); loginDlg_->setStatus(tr("已连接，请登录")); }
        });
        connect(tcp_, &TcpClient::disconnected, this, [this]{
            statusBar()->showMessage(tr("与服务器断开"));
            if (loginDlg_) { loginDlg_->setBusy(true); loginDlg_->setStatus(tr("未连接服务器"), true); }
        });
        connect(tcp_, &TcpClient::errorOccurred, this, [this](const QString& e){
            statusBar()->showMessage(tr("网络错误: %1").arg(e));
            if (loginDlg_) { loginDlg_->setBusy(true); loginDlg_->setStatus(tr("网络错误：%1").arg(e), true); }
        });
    }

    // 工单事件（此时 SessionWidget 可能尚未创建，使用前判空）
    if (orders_) {
        connect(orders_, &WorkOrderManager::inviteReceived, this, &MainWindow::showInvite);
        connect(orders_, &WorkOrderManager::joined, this, [this](const QString& oid){
            statusBar()->showMessage(tr("已加入工单: %1").arg(oid));
            if (session_) session_->setOrderId(oid);
        });
        connect(orders_, &WorkOrderManager::left, this, [this](const QString&){
            statusBar()->showMessage(tr("已离开工单"));
            if (session_) session_->setOrderId(QString());
        });
        connect(orders_, &WorkOrderManager::joinFailed, this, [this](const QString& reason){
            statusBar()->showMessage(tr("加入失败: %1").arg(reason));
        });
    }

    // 菜单：仅保留“历史工单”
    tickets_ = new TicketsClient(tcp_, this);
    auto* viewMenu = menuBar()->addMenu(tr("查看"));
    auto* actTickets = viewMenu->addAction(tr("历史工单"));
    connect(actTickets, &QAction::triggered, this, &MainWindow::showTickets);
    //菜单会议功能
    auto* viewMenu1 = menuBar()->addMenu(tr("在线会议"));
    auto* createMeeting = viewMenu1->addAction(tr("创建会议"));
    auto* joinMeet = viewMenu1->addAction(tr("加入会议"));
    connect(joinMeet, &QAction::triggered, this, &MainWindow::joinMeeting);
    // 启动即显示登录对话框
    showLogin();
    if (loginDlg_) {
        loginDlg_->setBusy(true);
        loginDlg_->setStatus(tr("正在连接服务器..."));
    }
}

void MainWindow::showLogin() {
    if (!loginDlg_) return;
    loginDlg_->show();
    loginDlg_->raise();
    loginDlg_->activateWindow();
}

void MainWindow::ensureSessionReady() {
    if (session_) return;
    session_ = new SessionWidget(this);
    setCentralWidget(session_);                     // 登录成功后替换中央窗口
    session_->bindServices(tcp_, auth_, orders_, telem_);
    if (chat_) session_->attachChat(chat_);         // 若聊天已注入，顺便绑定
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
    ensureSessionReady();                 // 登录成功后再创建并显示工单/设备界面
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

// 历史工单菜单动作
void MainWindow::showTickets() {
    if (!dlgTickets_) {
        dlgTickets_ = new TicketsViewDialog(this);
        dlgTickets_->bindServices(tickets_, orders_);
    }
    dlgTickets_->show();
    dlgTickets_->raise();
    dlgTickets_->activateWindow();
    dlgTickets_->refresh();
}

void MainWindow::joinMeeting(){
    MeetingDialog *dlg = new MeetingDialog("expert_user", "localhost", 12347, "", this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->show();
}
