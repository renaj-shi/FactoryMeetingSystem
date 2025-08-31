#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QSet>
#include <QHash>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>

// 你工程已有
#include "meetingroomdialog.h"

QT_BEGIN_NAMESPACE
class QThread;
class DeviceWorker;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 设备参数结构体
struct DeviceParams {
    double temperature{0.0};
    double pressure{0.0};
    double vibration{0.0};
    double current{0.0};
    double voltage{0.0};
    int    speed{0};
    bool   status{true};
    QDateTime lastUpdate;
};

// 工单状态枚举
enum TicketStatus {
    PENDING = 0,    // 待处理
    PROCESSING = 1, // 处理中
    RESOLVED = 2,   // 已解决
    CLOSED = 3      // 已关闭
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton1_clicked();  // 打开设备监控对话框
    void onFactoryNewConnection();
    void onExpertNewConnection();
    void onReadyRead();
    void onDisconnected();
    void on_pushButton2_clicked();  // 工单列表
    void on_meetingButton_clicked();
    void on_pushButton3_clicked();  // 启停设备模拟
    void showTicketsDialog();

private:
    // 业务辅助
    bool initDatabase();
    void handleLogin(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType);
    void handleRegister(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType);
    void handleCreateTicket(QTcpSocket *socket, const QStringList &parts);

    // 发送/广播
    void sendResponse(QTcpSocket *socket, const QString &response);
    void sendToExpert(QTcpSocket* s, const QString& line);
    void broadcastToExperts(const QString& line);
    void broadcastTelemetry(const QString& orderId, const QJsonObject& obj);
    void broadcastDeviceParams(const DeviceParams &params);

    // 新增：聊天与统一行解析
    void broadcastChat(const QString& orderId, const QString& from, const QString& text);
    void processLine(QTcpSocket* socket, const QString& line);

private:
    Ui::MainWindow *ui{nullptr};
    QTcpServer *factoryServer{nullptr};
    QTcpServer *expertServer{nullptr};
    QSqlDatabase db;

    // 在线客户端类型映射与账号映射
    QMap<QTcpSocket*, QString> clientTypeMap;              // socket -> FACTORY / EXPERT
    QMap<QTcpSocket*, QString> m_socketToUsername;         // socket -> 登录名
    QMap<QString, QTcpSocket*> m_factorySockets;           // 工厂用户名 -> socket

    // 专家/工厂在线/参与关系
    QSet<QTcpSocket*> expertSockets;                       // 在线专家集合
    QHash<QString, QSet<QTcpSocket*>> orderExperts;        // 工单 -> 专家socket集合
    QHash<QString, QSet<QTcpSocket*>> orderFactories;      // 工单 -> 工厂socket集合（聊天互通用）
    QHash<QString, QSet<QTcpSocket*>> teleSubs;            // 工单 -> 订阅设备数据的专家集合
    QHash<QTcpSocket*, QString> expertJoinedOrder;         // 专家 -> 已加入工单
    QHash<QTcpSocket*, QString> factoryJoinedOrder;        // 工厂 -> 已加入工单

    // 设备与会话
    DeviceWorker *m_deviceWorker{nullptr};
    QThread      *m_deviceThread{nullptr};
    MeetingRoomDialog *meetingRoom{nullptr};

    // 活跃工单（可选）
    QString activeOrderId_;
};

#endif // MAINWINDOW_H
