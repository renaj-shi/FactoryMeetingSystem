// ===================================================================
// [MOD] 文件名: main.cpp
// [MOD] 内容: 完整代码，保留了原有结构，并为本次修改添加了注释。
// ===================================================================
#include <QApplication>
#include "mainwindow.h"

#include "tcpclient.h"
#include "authmanager.h"
#include "workordermanager.h"
#include "telemetryclient.h"
#include "chatclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 服务对象（随 app 生命周期自动释放）
    auto* tcp    = new TcpClient(&app);
    auto* auth   = new AuthManager(tcp, &app);
    auto* orders = new WorkOrderManager(tcp, auth, &app);
    auto* telem  = new TelemetryClient(tcp, &app);
    auto* chat   = new ChatClient(tcp, auth, &app);

    MainWindow w;
    // 先绑定，再连接，避免过快连接错过 connected 信号
    w.bindServices(tcp, auth, orders, telem);
    w.attachChat(chat);    // 注入聊天模块
    w.show();

    // [DEL] 启动时自动连接服务器的逻辑已被移除。
    // [DEL] 新的逻辑是：当用户点击“立即登录”并弹出登录框时，由 MainWindow 主动发起连接。
    // const QString serverIp   = QStringLiteral("127.0.0.1");
    // const quint16 serverPort = 12346; // 专家端端口
    // tcp->connectTo(serverIp, serverPort);

    return app.exec();
}
