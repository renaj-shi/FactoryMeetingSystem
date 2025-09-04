// ===================================================================
// [MOD] 文件名: mainwindow.h
// [MOD] 内容: 修正后的完整代码。移除了不再需要的 showTickets() 槽和 dlgTickets_ 成员，
// [MOD] 并为新的“创建会议”按钮添加了对应的 createMeeting() 槽。
// ===================================================================
#pragma once

#include <QMainWindow>

class LoginDialog;
class SessionWidget;

// [ADD] 前向声明：新增欢迎页与注册对话框
class WelcomeWidget;   // [ADD]
class RegisterDialog;  // [ADD]

class TcpClient;
class AuthManager;
class WorkOrderManager;
class TelemetryClient;
class ChatClient;

class TicketsClient;
// [DEL] 不再需要前向声明 TicketsViewDialog，因为它已被 TicketsPageWidget 替代
// class TicketsViewDialog;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // 服务注入（登录成功后再创建 SessionWidget）
    void bindServices(TcpClient* tcp,
                      AuthManager* auth,
                      WorkOrderManager* orders,
                      TelemetryClient* telem);

    // 聊天模块注入
    void attachChat(ChatClient* chat);

public slots:
    // 工单邀请弹窗
    void showInvite(const QString& orderId, const QString& title);

private slots:
    void onLoginSucceeded(const QString& userId, const QString& token);
    void onLoginFailed(const QString& reason);

    // [DEL] 历史工单菜单动作的槽不再需要，功能已移入SessionWidget
    // void showTickets();
    void joinMeeting();
    void createMeeting(); // [ADD] 为“创建会议”按钮添加对应的槽函数

private:
    void setupUiEx();        // 初始UI（不创建 SessionWidget）
    void showLogin();        // 弹出登录对话框
    void ensureSessionReady(); // 登录成功后懒创建 SessionWidget 并绑定服务

private:
    Ui::MainWindow* ui{nullptr};

    // UI 组件
    LoginDialog*   loginDlg_{nullptr};
    SessionWidget* session_{nullptr};

    // [ADD] 欢迎页与注册对话框
    WelcomeWidget*  welcome_{nullptr};     // [ADD]
    RegisterDialog* regDlg_{nullptr};      // [ADD]

    // 服务对象（外部注入）
    TcpClient*       tcp_{nullptr};
    AuthManager*     auth_{nullptr};
    WorkOrderManager* orders_{nullptr};
    TelemetryClient* telem_{nullptr};
    ChatClient*      chat_{nullptr};

    // [MOD] TicketsClient 实例仍然需要，因为它要被传递给SessionWidget
    TicketsClient*     tickets_{nullptr};
    // [DEL] 历史工单对话框的指针不再需要
    // TicketsViewDialog* dlgTickets_{nullptr};

    // [ADD] 记录上次注册提交的账号，用于回填登录框
    QString pendingRegUser_; // [ADD]
    QString pendingRegPwd_;  // [ADD]
};
