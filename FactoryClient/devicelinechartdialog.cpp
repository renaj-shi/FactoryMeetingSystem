#include "devicelinechartdialog.h"
#include "ui_devicelinechartdialog.h"

#include <QDebug>
#include <QRandomGenerator>

// 生成随机颜色
QColor generateRandomColor(int seed) {
    QRandomGenerator rng(seed);
    return QColor(
        rng.bounded(100, 255),
        rng.bounded(100, 255),
        rng.bounded(100, 255)
    );
}

DeviceLineChartDialog::DeviceLineChartDialog(const DeviceInfo &device, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeviceLineChartDialog)
    , m_device(device)
    , lastUpdateTime_(0)
{
    ui->setupUi(this);
    
    // 保存设备信息
    
    // 设置窗口标题和设备名称
    setWindowTitle(QString("设备数据折线图 - %1").arg(device.deviceName()));
    ui->deviceNameLabel->setText(QString("设备名称: %1 (ID: %2)").arg(device.deviceName()).arg(device.deviceId()));
    
    // 初始化图表
    initChart();
    
    // 创建参数按钮
    createParameterButtons();
    
    // 连接信号槽
    connect(ui->clearDataButton, &QPushButton::clicked, this, &DeviceLineChartDialog::onClearDataClicked);
    
    // 设置初始更新时间
    lastUpdateTime_ = QDateTime::currentMSecsSinceEpoch();
    
    // 设置窗口大小和位置
    resize(800, 600);
}

DeviceLineChartDialog::~DeviceLineChartDialog()
{
    delete ui;
    parameterSeries_.clear();
}

void DeviceLineChartDialog::buildUI()
{
    // UI已经在.ui文件中定义，这里可以添加额外的UI设置
}

void DeviceLineChartDialog::initChart()
{
    // 创建图表
    chart_ = new QtCharts::QChart();
    chart_->setTitle("设备参数实时曲线");
    chart_->setAnimationOptions(QtCharts::QChart::NoAnimation); // 关闭动画
    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignBottom);
    
    // 创建X轴（时间轴）
    axisX_ = new QtCharts::QDateTimeAxis();
    axisX_->setFormat("hh:mm:ss");
    axisX_->setTitleText("时间");
    axisX_->setTickCount(7); // 稳定刻度数量
    chart_->addAxis(axisX_, Qt::AlignBottom);
    
    // 创建Y轴（数值轴）
    axisY_ = new QtCharts::QValueAxis();
    axisY_->setLabelFormat("%.1f");
    axisY_->setTitleText("数值");
    axisY_->setTickCount(6); // 稳定刻度数量
    chart_->addAxis(axisY_, Qt::AlignLeft);
    
    // 用ui里的占位QWidget作为容器
    QWidget* container = ui->chartView; // 这是普通QWidget
    auto lay = new QVBoxLayout(container);
    lay->setContentsMargins(0,0,0,0);
    
    chartView_ = new QtCharts::QChartView(chart_, container);
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate); // 优化视图更新
    lay->addWidget(chartView_);
    
    // 断言，确保chartView_不为nullptr
    Q_ASSERT(chartView_ && "chartView_ is null, check your .ui or promotion to QChartView");
    
    // 为每个参数创建折线系列
    int colorSeed = 0;
    for (const auto &param : m_device.parameters()) {
        QString paramName = param.name();
        
        // 创建折线系列
        QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
        series->setName(QString("%1 (%2)").arg(paramName).arg(param.unit()));
        series->setColor(generateRandomColor(colorSeed++));
        series->setVisible(false); // 初始隐藏
        
        // 添加到图表
        chart_->addSeries(series);
        series->attachAxis(axisX_);
        series->attachAxis(axisY_);
        
        // 保存到映射中
        parameterSeries_[paramName] = series;
    }
    
    // 设置初始坐标轴范围
    QDateTime now = QDateTime::currentDateTime();
    axisY_->setRange(-10, 10);
}

void DeviceLineChartDialog::createParameterButtons()
{
    QHBoxLayout *layout = ui->parameterButtonsLayout;
    
    // 清除现有的按钮
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            layout->removeWidget(item->widget());
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // 为每个参数创建按钮
    for (const auto &param : m_device.parameters()) {
        QString paramName = param.name();
        
        QPushButton *button = new QPushButton(QString("%1").arg(paramName));
        button->setCheckable(true);
        button->setChecked(false);
        button->setMinimumWidth(100);
        
        // 连接信号槽
        connect(button, &QPushButton::toggled, this, [=](bool checked) {
            onParameterButtonToggled(checked);
        });
        
        // 添加到布局
        layout->addWidget(button);
        
        // 保存到映射中
        parameterButtons_[paramName] = button;
    }
    
    // 添加弹性空间
    layout->addStretch();
}

void DeviceLineChartDialog::updateChartData(const DeviceInfo &device)
{
    // 更新设备信息
    m_device = device;
    
    // 获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 currentTimeMs = currentTime.toMSecsSinceEpoch();
    
    // 限制更新频率（每100ms更新一次）
    if (currentTimeMs - lastUpdateTime_ < 100) {
        return;
    }
    
    lastUpdateTime_ = currentTimeMs;
    
    // 更新每个参数的数据点
    for (const auto &param : device.parameters()) {
        QString paramName = param.name();
        
        // 检查参数系列是否存在
        if (!parameterSeries_.contains(paramName)) {
            continue;
        }
        
        QtCharts::QLineSeries *series = parameterSeries_[paramName];
        
        // 添加新的数据点
        series->append(currentTime.toMSecsSinceEpoch(), param.currentValue());
        
        // 限制数据点数量
        if (series->count() > maxPoints_) {
            const int extra = series->count() - maxPoints_;
            series->removePoints(0, extra);
        }
    }
    
    // 刷新坐标轴范围
    refreshAxisRange();
}

void DeviceLineChartDialog::refreshAxisRange()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    // X 轴滑动窗口
    if (!xInit_) {
        axisX_->setRange(QDateTime::fromMSecsSinceEpoch(nowMs - windowMs_), 
                         QDateTime::fromMSecsSinceEpoch(nowMs));
        xInit_ = true;
    } else {
        const qint64 oldMax = axisX_->max().toMSecsSinceEpoch();
        const qint64 dt = nowMs - oldMax;
        if (dt > 0) {
            // 用 scroll 平移，避免重算布局
            const double dx = chart_->plotArea().width() * (double)dt / (double)windowMs_;
            chart_->scroll(dx, 0);
            // 对齐边界，防止累计误差
            axisX_->setMax(QDateTime::fromMSecsSinceEpoch(nowMs));
            axisX_->setMin(QDateTime::fromMSecsSinceEpoch(nowMs - windowMs_));
        }
    }

    // 获取所有可见系列的Y轴范围
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    bool hasData = false;
    
    for (auto series : parameterSeries_) {
        if (series->isVisible() && series->count() > 0) {
            QList<QPointF> points = series->points();
            for (const auto &point : points) {
                double y = point.y();
                minY = qMin(minY, y);
                maxY = qMax(maxY, y);
            }
            hasData = true;
        }
    }
    
    if (hasData) {
        // 添加10%的边距
        const double margin = qMax((maxY - minY) * 0.1, 1.0);
        const double targetMin = minY - margin;
        const double targetMax = maxY + margin;
        
        // 获取当前Y轴范围
        const double curMin = axisY_->min();
        const double curMax = axisY_->max();
        
        // 只在越界时扩展
        double newMin = curMin;
        double newMax = curMax;
        if (targetMin < curMin) newMin = targetMin;
        if (targetMax > curMax) newMax = targetMax;
        
        if (newMin != curMin || newMax != curMax) {
            axisY_->setRange(newMin, newMax);
        }
    }
}

void DeviceLineChartDialog::onParameterButtonToggled(bool checked)
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }
    
    // 查找对应的参数名称
    QString paramName;
    for (auto it = parameterButtons_.begin(); it != parameterButtons_.end(); ++it) {
        if (it.value() == button) {
            paramName = it.key();
            break;
        }
    }
    
    // 更新系列可见性
    if (parameterSeries_.contains(paramName)) {
        parameterSeries_[paramName]->setVisible(checked);
        refreshAxisRange();
    }
}

void DeviceLineChartDialog::onClearDataClicked()
{
    // 清除所有系列的数据
    for (auto series : parameterSeries_) {
        series->clear();
    }
    
    // 重置坐标轴范围
    QDateTime now = QDateTime::currentDateTime();
    axisX_->setRange(now.addSecs(-60), now);
    axisY_->setRange(-10, 10);
}