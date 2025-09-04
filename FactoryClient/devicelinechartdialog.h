#ifndef DEVICELINECHARTDIALOG_H
#define DEVICELINECHARTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>

#include "deviceinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DeviceLineChartDialog; }
QT_END_NAMESPACE

// 设备折线图对话框类
class DeviceLineChartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceLineChartDialog(const DeviceInfo &device, QWidget *parent = nullptr);
    ~DeviceLineChartDialog();

    // 更新图表数据
    void updateChartData(const DeviceInfo &device);

private:
    // 构建UI界面
    void buildUI();
    
    // 初始化图表
    void initChart();
    
    // 创建参数选择按钮
    void createParameterButtons();
    
    // 刷新坐标轴范围
    void refreshAxisRange();

private slots:
    // 处理参数选择按钮点击
    void onParameterButtonToggled(bool checked);
    
    // 处理清除数据按钮点击
    void onClearDataClicked();

private:
    Ui::DeviceLineChartDialog *ui;
    
    // 设备信息
    DeviceInfo m_device;
    
    // 图表组件
    QtCharts::QChartView* chartView_;
    QtCharts::QChart* chart_;
    QtCharts::QDateTimeAxis* axisX_;
    QtCharts::QValueAxis* axisY_;
    
    // 参数曲线系列
    QMap<QString, QtCharts::QLineSeries*> parameterSeries_;
    
    // 参数按钮映射
    QMap<QString, QPushButton*> parameterButtons_;
    
    // 最大数据点数
    const int maxPoints_ = 300;
    
    // 上次更新时间
    qint64 lastUpdateTime_;
    
    // 窗口时间范围（毫秒）
    qint64 windowMs_ = 60 * 1000;
    
    // X轴初始化标志
    bool xInit_ = false;
};

#endif // DEVICELINECHARTDIALOG_H