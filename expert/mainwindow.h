#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LoginDialog;
class SessionWidget;

class TcpClient;
class AuthManager;
class WorkOrderManager;
class TelemetryClient;
class ChatClient;

class TicketsClient;
class TicketsViewDialog;

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

    // 历史工单菜单动作
    void showTickets();
    void joinMeeting();
private:
    void setupUiEx();        // 初始UI（不创建 SessionWidget）
    void showLogin();        // 弹出登录对话框
    void ensureSessionReady(); // 登录成功后懒创建 SessionWidget 并绑定服务

private:
    Ui::MainWindow* ui{nullptr};

    // UI 组件
    LoginDialog*   loginDlg_{nullptr};
    SessionWidget* session_{nullptr};

    // 服务对象（外部注入）
    TcpClient*       tcp_{nullptr};
    AuthManager*     auth_{nullptr};
    WorkOrderManager* orders_{nullptr};
    TelemetryClient* telem_{nullptr};
    ChatClient*      chat_{nullptr};

    // 历史工单
    TicketsClient*     tickets_{nullptr};
    TicketsViewDialog* dlgTickets_{nullptr};
};

#endif // MAINWINDOW_H
