#ifndef MAININTERFACEDIALOG_H
#define MAININTERFACEDIALOG_H

#include <QDialog>
#include <QTabBar>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QMap>
#include <QStringList>
#include <QTcpSocket>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTimer>
#include <QRadioButton>
#include <QButtonGroup>
#include "devicemonitorpanel.h"
#include <QDateTime>
// 【增】添加所需头文件
#include "meetingdialog.h"
// 设备参数结构体
struct DeviceParams {
    double temperature;      // 温度
    double pressure;         // 压力
    double vibration;        // 振动
    double current;          // 电流
    double voltage;          // 电压
    double speed;            // 转速
    int status;              // 设备状态
    QDateTime lastUpdate;    // 最后更新时间
};

// 前向声明
class WorkOrderDialog;
class ChatDialog;

namespace Ui {
class MainInterfaceDialog;
}

class MainInterfaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainInterfaceDialog(QWidget *parent = nullptr);
    ~MainInterfaceDialog();
    // 【增】添加注入连接信息的公共方法
    void setConnection(QTcpSocket* socket, const QString& username);

private slots:
    // 侧边栏按钮点击事件
    void on_loginButton_clicked();
    void on_workOrderButton_clicked();
    void on_newWorkOrderButton_clicked();
    void on_workOrderListButton_clicked();
    void on_communicationButton_clicked();
    void on_textMessageButton_clicked();
    void on_joinMeetingButton_clicked();
    void on_deviceMonitorButton_clicked();
    void on_realTimeDataButton_clicked();
    void on_deviceRecordingButton_clicked();
    void on_knowledgeBaseButton_clicked();

    // 标签栏事件
    void on_tabBar_currentChanged(int index);
    void on_tabBar_tabCloseRequested(int index);

    // 【增】添加新的私有槽函数
    void onSocketReadyRead();
    void onCreateTicketClicked();
    void updateDeviceMonitorData(const QString &deviceId, double temperature, double pressure, double vibration, double current, double voltage, double speed, int status);
    void startDeviceMonitorTimer();
    void stopDeviceMonitorTimer();
private:
    Ui::MainInterfaceDialog *ui;
    QMap<QString, int> m_featurePageMap; // 功能名称到页面索引的映射
    QStringList m_tabFeatureNames; // 标签对应的功能名称列表
    // 【增】添加新的私有成员
    QTcpSocket* m_socket = nullptr;
    QString     m_username;
    QByteArray  m_recvBuf; // 接收缓冲区，用于处理没有换行符的消息
    QTimer*     m_netPump = nullptr; // 网络数据轮询泵
    QVBoxLayout* m_sidebarLayout = nullptr;
    QComboBox*  newPrioBox_   = nullptr; // 保留用于向后兼容
    QLineEdit*  newTitleEdit_ = nullptr;
    QTextEdit*  newDescEdit_ = nullptr;
    QLabel* newResultLabel_= nullptr;
    DeviceMonitorPanel* m_deviceMonitorPanel = nullptr; // 指向设备监控面板的指针
    WorkOrderDialog* m_workOrderDialog = nullptr; // 指向新建工单对话框的指针
    ChatDialog* m_chatDialog = nullptr; // 指向聊天对话框的指针
    QMap<QString, DeviceParams> m_deviceDataCache; // 设备数据缓存，用于实时页未创建时存储数据
    MeetingDialog *meetingDialog;

    void setupUI(); // 设置UI
    void createSidebar(); // 创建侧边栏
    void createTopBar(); // 创建顶部栏
    void createCentralWidget(); // 创建中央部件
    void createEmptyStatePage(); // 创建空状态页面
    void createAndShowFeaturePage(const QString &featureName, const QString &displayName); // 创建并显示功能页面
    QWidget *createFeatureContentWidget(const QString &featureName, const QString &displayName); // 创建功能内容部件
    // 辅助函数：从JSON字符串中提取指定键的值
    QString getValueFromJson(const QString &json, const QString &key, const QString &defaultValue = "0");
    // 检查并应用缓存的设备数据
    void applyDeviceDataCache();
};

#endif // MAININTERFACEDIALOG_H
