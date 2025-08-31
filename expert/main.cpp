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

    // 专家端服务器地址/端口（根据你的后端实际修改）
    const QString serverIp   = QStringLiteral("127.0.0.1");
    const quint16 serverPort = 12346; // 专家端端口
    tcp->connectTo(serverIp, serverPort);

    return app.exec();
}
