#ifndef DEVICEMONITORPANEL_H
#define DEVICEMONITORPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>
#include "deviceinfo.h"

namespace Ui {
class DeviceMonitorPanel;
}

// 前向声明
class QLabel;
class QFrame;
class QPropertyAnimation;
class QTimer;

// 设备监控面板类
class DeviceMonitorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceMonitorPanel(QWidget *parent = nullptr);
    ~DeviceMonitorPanel();

    // 初始化UI
    void initialize();
    
    // 获取设备管理器的引用
    DeviceManager& getDeviceManager() { return m_deviceManager; }
    
    // 控制定时器
    void startTimer();
    void stopTimer();
    
protected:
    // 事件过滤器
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private slots:
    // 设备栏点击事件
    void onDeviceBarClicked(const QString &deviceId);
    
public:
    // 定时更新数据 - 移到public以便外部调用
    void updateData();
    
    // 更新设备的最后参数接收时间
    void updateLastParameterTime(const QString &deviceId);
    
private:
    Ui::DeviceMonitorPanel *ui;
    
    // 创建设备栏
    QWidget* createDeviceBar(const DeviceInfo &device);
    
    // 创建参数抽屉
    QWidget* createParameterDrawer(const DeviceInfo &device);
    
    // 更新设备栏状态
    void updateDeviceBar(const QString &deviceId);
    
    // 更新参数抽屉
    void updateParameterDrawer(const QString &deviceId);
    
    // 获取状态对应的颜色样式
    QString getStatusStyle(DeviceStatus status);
    
    // 获取状态对应的文本
    QString getStatusText(DeviceStatus status);
    
    // 获取参数值的颜色样式（根据是否在正常范围内）
    QString getParameterValueStyle(const DeviceParameter &param);
    
    // 设备管理器
    DeviceManager m_deviceManager;
    
    // 主布局
    QVBoxLayout *m_mainLayout;
    
    // 设备栏和参数抽屉的映射
    QMap<QString, QWidget*> m_deviceBars;
    QMap<QString, QWidget*> m_parameterDrawers;
    QMap<QString, QPropertyAnimation*> m_drawerAnimations;
    QMap<QString, bool> m_drawerVisibleStates;
    
    // 数据更新定时器
    QTimer *m_updateTimer;
    
    // 跟踪上次收到参数的时间
    QMap<QString, qint64> m_lastParameterUpdateTime;
    
    // 超时时间（毫秒）
    const int m_timeoutThreshold = 2000;
};

#endif // DEVICEMONITORPANEL_H