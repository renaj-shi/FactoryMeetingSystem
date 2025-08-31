#pragma once
#include <QMainWindow>

class LoginDialog;
class SessionWidget;

class TcpClient;
class AuthManager;
class WorkOrderManager;
class TelemetryClient;

class TicketsClient;
class KnowledgeClient;
class TicketsViewDialog;
class KnowledgeViewDialog;

class ChatClient;
namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow() override;

    // 绑定服务（注意：SessionWidget 会在登录成功后才创建并绑定）
    void bindServices(TcpClient* tcp,
                      AuthManager* auth,
                      WorkOrderManager* orders,
                      TelemetryClient* telem);

    void attachChat(ChatClient* chat);

public slots:
    // 工单邀请弹窗（S->C: ORDER|INVITE|orderId|title 时调用）
    void showInvite(const QString& orderId, const QString& title);

private slots:
    void onLoginSucceeded(const QString& userId, const QString& token);
    void onLoginFailed(const QString& reason);

    void showTickets();
    void showKnowledge();

private:
    void setupUiEx();       // 初始 UI（不创建 SessionWidget）
    void showLogin();       // 弹出登录对话框
    void ensureSessionReady(); // 懒创建 SessionWidget 并绑定服务

    Ui::MainWindow* ui{nullptr};
    LoginDialog* loginDlg_{nullptr};
    SessionWidget* session_{nullptr};    // 登录成功后才创建

    TcpClient* tcp_{nullptr};
    AuthManager* auth_{nullptr};
    WorkOrderManager* orders_{nullptr};
    TelemetryClient* telem_{nullptr};

    TicketsClient* tickets_{nullptr};
    KnowledgeClient* kb_{nullptr};
    TicketsViewDialog* dlgTickets_{nullptr};
    KnowledgeViewDialog* dlgKB_{nullptr};

    ChatClient* chat_{nullptr};
};
