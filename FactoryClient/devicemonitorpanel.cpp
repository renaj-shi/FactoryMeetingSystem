#include "devicemonitorpanel.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDateTime>
#include <QHeaderView>
#include <QTableWidget>
#include <QScrollArea>
#include <QEvent>
#include <QDebug>

DeviceMonitorPanel::DeviceMonitorPanel(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_updateTimer(new QTimer(this)),
      m_timeoutThreshold(2000)
{
    // 设置窗口大小策略，使其能够占满父容器
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 设置布局
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 初始化定时器
    connect(m_updateTimer, &QTimer::timeout, this, &DeviceMonitorPanel::updateData);
    // 不自动启动定时器，等待外部触发
    // m_updateTimer->start(1000); // 1秒更新一次数据
    
    // 初始化UI
    initialize();
}

// 事件过滤器，用于捕获设备栏的点击事件
bool DeviceMonitorPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && widget->property("deviceId").isValid()) {
            QString deviceId = widget->property("deviceId").toString();
            onDeviceBarClicked(deviceId);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

DeviceMonitorPanel::~DeviceMonitorPanel()
{
    delete m_updateTimer;
}

void DeviceMonitorPanel::initialize()
{
    // 创建滚动区域作为主容器
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; }"
        "QScrollBar:vertical { background: #f1f5f9; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    
    // 设置滚动区域大小策略为扩展
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 创建滚动区域的内容widget
    QWidget *scrollContent = new QWidget();
    scrollContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(0);
    
    // 添加标题
    QLabel *titleLabel = new QLabel("设备监控面板", scrollContent);
    titleLabel->setStyleSheet(
        "font-size: 20px; "
        "font-weight: bold; "
        "color: #1e293b; "
        "margin-bottom: 20px; "
        "margin-top: 10px;"
    );
    scrollLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    
    // 创建设备栏和参数抽屉
    for (const auto &device : m_deviceManager.devices()) {
        // 创建设备栏
        QWidget *deviceBar = createDeviceBar(device);
        m_deviceBars[device.deviceId()] = deviceBar;
        scrollLayout->addWidget(deviceBar);
        
        // 创建参数抽屉（初始隐藏）
        QWidget *parameterDrawer = createParameterDrawer(device);
        parameterDrawer->setMaximumHeight(0);
        parameterDrawer->setMinimumHeight(0);
        parameterDrawer->hide();
        m_parameterDrawers[device.deviceId()] = parameterDrawer;
        scrollLayout->addWidget(parameterDrawer);
        
        // 创建动画
        QPropertyAnimation *animation = new QPropertyAnimation(parameterDrawer, "maximumHeight");
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        m_drawerAnimations[device.deviceId()] = animation;
        
        // 设置初始状态
        m_drawerVisibleStates[device.deviceId()] = false;
        
        // 初始化最后更新时间为当前时间减去4秒，确保如果没有收到数据，进入界面时立即显示未连接状态
        m_lastParameterUpdateTime[device.deviceId()] = QDateTime::currentMSecsSinceEpoch() - 4000;
    }
    
    scrollLayout->addStretch();
    
    // 设置滚动区域内容
    scrollArea->setWidget(scrollContent);
    m_mainLayout->addWidget(scrollArea);
}

QWidget* DeviceMonitorPanel::createDeviceBar(const DeviceInfo &device)
{
    QWidget *deviceBar = new QWidget(this);
    deviceBar->setStyleSheet(
        "background-color: #f8fafc; "
        "border-bottom: 1px solid #e2e8f0; "
        "padding: 15px;"
    );
    deviceBar->setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout *layout = new QHBoxLayout(deviceBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    // 设备名称
    QLabel *nameLabel = new QLabel(device.deviceName(), deviceBar);
    nameLabel->setStyleSheet(
        "font-size: 16px; "
        "font-weight: medium; "
        "color: #334155;"
    );
    layout->addWidget(nameLabel);
    
    // 状态指示器
    QLabel *statusLabel = new QLabel(getStatusText(device.status()), deviceBar);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet(
        getStatusStyle(device.status()) + 
        "font-size: 14px; "
        "font-weight: bold;"
    );
    layout->addWidget(statusLabel);
    
    // 添加伸缩项，确保设备名称和状态指示器靠左显示
    layout->addStretch();
    
    // 箭头指示器（放在最右边）
    QLabel *arrowLabel = new QLabel("▼", deviceBar);
    arrowLabel->setObjectName("arrowLabel");
    arrowLabel->setStyleSheet(
        "color: #64748b; "
        "font-size: 12px;"
    );
    arrowLabel->setVisible(true); // 初始显示倒三角箭头
    layout->addWidget(arrowLabel);
    
    // 安装事件过滤器以捕获点击事件
    deviceBar->installEventFilter(this);
    deviceBar->setProperty("deviceId", device.deviceId());
    
    return deviceBar;
}

QWidget* DeviceMonitorPanel::createParameterDrawer(const DeviceInfo &device)
{
    QWidget *drawer = new QWidget(this);
    drawer->setStyleSheet(
        "background-color: #f1f5f9; "
        "border-bottom: 1px solid #e2e8f0; "
        "padding: 15px;"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(drawer);
    
    // 参数表格
    QTableWidget *parameterTable = new QTableWidget(drawer);
    parameterTable->setObjectName("parameterTable");
    parameterTable->setColumnCount(4);
    parameterTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    parameterTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 隐藏默认表头
    parameterTable->horizontalHeader()->setVisible(false);
    parameterTable->verticalHeader()->setVisible(false); // 隐藏行号
    
    // 设置表格样式
    parameterTable->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: 1px solid #cbd5e1;"
        "    gridline-color: #cbd5e1;"
        "}"
        "QTableWidgetItem {"
        "    padding: 10px;"
        "    border: 1px solid #cbd5e1;"
        "}"
        "QTableWidgetItem:selected {"
        "    background-color: #e2e8f0;"
        "}"
    );
    
    // 设置表格列宽
    parameterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 设置表格的大小策略，确保它可以正确扩展
    parameterTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    parameterTable->setMinimumHeight(100); // 设置最小高度，确保表格有足够空间显示
    
    // 设置表格列宽
    parameterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 设置行高
    parameterTable->verticalHeader()->setDefaultSectionSize(36);
    
    // 填充参数数据，行数量为设备参数数量+1（+1表示表头行）
    parameterTable->setRowCount(device.parameters().size() + 1);
    
    // 添加表头行
    QStringList headers = {"参数名称", "当前值", "单位", "阈值范围"};
    for (int col = 0; col < headers.size(); col++) {
        // 明确创建表头文本，确保第二列是"当前值"
        QString headerText = (col == 1) ? "当前值" : headers[col];
        QTableWidgetItem *headerItem = new QTableWidgetItem(headerText);
        
        // 设置表头样式
        QFont font = headerItem->font();
        font.setBold(true);
        headerItem->setFont(font);
        
        // 改为暗白色背景 (#f1f5f9)
        headerItem->setBackgroundColor(QColor(241, 245, 249));
        
        // 改为深色文字
        headerItem->setTextColor(QColor(51, 65, 85));
        
        // 设置为不可编辑
        headerItem->setFlags(headerItem->flags() & ~Qt::ItemIsEditable);
        
        // 添加到表格
        parameterTable->setItem(0, col, headerItem);
    }
    
    // 填充参数数据（从第二行开始）
    int row = 1;
    for (const auto &param : device.parameters()) {
        // 参数名称
        QTableWidgetItem *nameItem = new QTableWidgetItem(param.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        parameterTable->setItem(row, 0, nameItem);
        
        // 当前值
        QTableWidgetItem *valueItem = new QTableWidgetItem(QString::number(param.currentValue(), 'f', 1));
        // 使用setForeground代替setStyleSheet
        QColor textColor;
        if (param.isNormal()) {
            textColor = QColor(34, 197, 94); // 绿色
        } else {
            textColor = QColor(239, 68, 68); // 红色
        }
        valueItem->setForeground(QBrush(textColor));
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        parameterTable->setItem(row, 1, valueItem);
        
        // 单位
        QTableWidgetItem *unitItem = new QTableWidgetItem(param.unit());
        unitItem->setFlags(unitItem->flags() & ~Qt::ItemIsEditable);
        parameterTable->setItem(row, 2, unitItem);
        
        // 阈值范围
        QTableWidgetItem *rangeItem = new QTableWidgetItem(QString("%1-%2%3").arg(param.minValue()).arg(param.maxValue()).arg(param.unit()));
        rangeItem->setFlags(rangeItem->flags() & ~Qt::ItemIsEditable);
        parameterTable->setItem(row, 3, rangeItem);
        
        row++;
    }
    
    layout->addWidget(parameterTable);
    
    return drawer;
}

void DeviceMonitorPanel::onDeviceBarClicked(const QString &deviceId)
{
    bool isVisible = m_drawerVisibleStates[deviceId];
    QWidget *drawer = m_parameterDrawers[deviceId];
    QPropertyAnimation *animation = m_drawerAnimations[deviceId];
    QWidget *deviceBar = m_deviceBars[deviceId];
    
    if (isVisible) {
        // 隐藏抽屉 - 先重置高度限制以允许平滑动画
        drawer->setMaximumHeight(10000);
        drawer->setMinimumHeight(0);
        animation->setStartValue(drawer->height());
        animation->setEndValue(0);
        
        // 更新箭头为倒三角（表示可展开）
        QLabel *arrowLabel = deviceBar->findChild<QLabel*>("arrowLabel");
        if (arrowLabel) {
            arrowLabel->setText("▼"); // 倒三角
            arrowLabel->setVisible(true); // 始终显示箭头
        }
    } else {
        // 显示抽屉
        drawer->show();
        drawer->setMaximumHeight(10000); // 临时设置一个大值以获取实际大小
        drawer->setMinimumHeight(0);
        
        // 获取表格并计算精确高度
        QTableWidget *parameterTable = drawer->findChild<QTableWidget*>("parameterTable");
        int rowHeight = 36; // 每行高度
        int headerHeight = 40; // 表头高度
        int padding = 30; // 抽屉内边距
        
        int tableHeight = 0;
        if (parameterTable) {
            // 精确计算表格所需高度：行数 × 行高 + 表头高度
            tableHeight = parameterTable->rowCount() * rowHeight + headerHeight + padding;
        }
        
        // 确保有最小高度
        int targetHeight = qMax(100, tableHeight);
        
        animation->setStartValue(0);
        animation->setEndValue(targetHeight);
        
        // 更新箭头为正三角（表示可收起）
        QLabel *arrowLabel = deviceBar->findChild<QLabel*>("arrowLabel");
        if (arrowLabel) {
            arrowLabel->setText("▲"); // 正三角
            arrowLabel->setVisible(true); // 始终显示箭头
        }
    }
    
    // 断开之前的连接，然后重新连接动画完成信号以更新状态
    disconnect(animation, &QPropertyAnimation::finished, nullptr, nullptr);
    connect(animation, &QPropertyAnimation::finished, [this, deviceId, isVisible, drawer]() {
        if (isVisible) {
            drawer->hide();
            drawer->setMaximumHeight(0); // 隐藏时重置最大高度
        } else {
            // 显示完成后固定抽屉高度为表格所需高度
            QTableWidget *parameterTable = drawer->findChild<QTableWidget*>("parameterTable");
            int rowHeight = 36; // 每行高度
            int headerHeight = 40; // 表头高度
            int padding = 30; // 抽屉内边距
            
            int tableHeight = 0;
            if (parameterTable) {
                // 精确计算表格所需高度：行数 × 行高 + 表头高度
                tableHeight = parameterTable->rowCount() * rowHeight + headerHeight + padding;
            }
            
            // 确保有最小高度
            int targetHeight = qMax(100, tableHeight);
            
            drawer->setMaximumHeight(targetHeight);
            drawer->setMinimumHeight(targetHeight);
        }
        m_drawerVisibleStates[deviceId] = !isVisible;
        
        // 强制更新主滚动区域，确保滚动条正确显示
        this->updateGeometry();
        QScrollArea *scrollArea = this->findChild<QScrollArea*>();
        if (scrollArea) {
            scrollArea->updateGeometry();
            scrollArea->widget()->updateGeometry();
        }
    });
    
    animation->start();
}

void DeviceMonitorPanel::updateData()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    for (const auto &device : m_deviceManager.devices()) {
        QString deviceId = device.deviceId();
        
        // 检查是否超时
        if (currentTime - m_lastParameterUpdateTime[deviceId] > m_timeoutThreshold) {
            // 超时，标记为未连接状态
            DeviceInfo* mutableDevice = m_deviceManager.getDevice(deviceId);
            if (mutableDevice) {
                mutableDevice->setStatus(DeviceStatus::DISCONNECTED);
            }
        }
        
        updateDeviceBar(deviceId);
        // 无论抽屉是否可见，都更新参数数据，确保打开时能看到最新值
        updateParameterDrawer(deviceId);
    }
}

void DeviceMonitorPanel::updateLastParameterTime(const QString &deviceId)
{
    // 更新设备的最后参数接收时间为当前时间
    m_lastParameterUpdateTime[deviceId] = QDateTime::currentMSecsSinceEpoch();
    
    // 当收到新参数时，将设备状态设置为正常
    DeviceInfo* device = m_deviceManager.getDevice(deviceId);
    if (device && device->status() == DeviceStatus::DISCONNECTED) {
        device->setStatus(DeviceStatus::NORMAL);
    }
}

void DeviceMonitorPanel::startTimer()
{
    // 启动定时器
    if (m_updateTimer && !m_updateTimer->isActive()) {
        m_updateTimer->start(1000); // 每秒更新一次
    }
}

void DeviceMonitorPanel::stopTimer()
{
    // 停止定时器
    if (m_updateTimer && m_updateTimer->isActive()) {
        m_updateTimer->stop();
    }
}

void DeviceMonitorPanel::updateDeviceBar(const QString &deviceId)
{
    DeviceInfo *device = m_deviceManager.getDevice(deviceId);
    if (!device) return;
    
    QWidget *deviceBar = m_deviceBars[deviceId];
    if (!deviceBar) return;
    
    // 更新状态
    QLabel *statusLabel = deviceBar->findChild<QLabel*>("statusLabel");
    if (statusLabel) {
        statusLabel->setText(getStatusText(device->status()));
        statusLabel->setStyleSheet(
            getStatusStyle(device->status()) + 
            "font-size: 14px; "
            "font-weight: bold;"
        );
    }
}

void DeviceMonitorPanel::updateParameterDrawer(const QString &deviceId)
{
    // 输出调试信息，确认此函数被调用
    qDebug() << "[DeviceMonitor] 尝试更新设备参数抽屉，设备ID:" << deviceId;
    
    DeviceInfo *device = m_deviceManager.getDevice(deviceId);
    if (!device) {
        qDebug() << "[DeviceMonitor] 未找到设备，设备ID:" << deviceId;
        return;
    }
    
    QWidget *drawer = m_parameterDrawers[deviceId];
    if (!drawer) {
        qDebug() << "[DeviceMonitor] 未找到参数抽屉，设备ID:" << deviceId;
        return;
    }
    
    QTableWidget *parameterTable = drawer->findChild<QTableWidget*>("parameterTable");
    if (!parameterTable) {
        qDebug() << "[DeviceMonitor] 未找到参数表格，设备ID:" << deviceId;
        return;
    }
    
    // 检查设备是否处于未连接状态
    bool isDisconnected = (device->status() == DeviceStatus::DISCONNECTED);
    
    // 更新参数值
    int row = 1; // 从第二行开始更新（第一行是表头）
    for (const auto &param : device->parameters()) {
        QTableWidgetItem *valueItem = parameterTable->item(row, 1);
        
        // 确定要显示的参数值
        double displayValue = param.currentValue();
        if (isDisconnected) {
            // 未连接状态下显示初始参数值（0.0）
            displayValue = 0.0;
        }
        
        if (valueItem) {
            // 输出调试信息，显示当前参数值
            qDebug() << "[DeviceMonitor] 更新参数:'" << param.name() << "' 值:" << displayValue;
            
            valueItem->setText(QString::number(displayValue, 'f', 1));
            // 使用setForeground代替setStyleSheet
            QColor textColor;
            if (isDisconnected) {
                // 未连接状态下文本颜色为红色
                textColor = QColor(239, 68, 68); // 红色
            } else if (param.isNormal()) {
                textColor = QColor(34, 197, 94); // 绿色
            } else {
                textColor = QColor(239, 68, 68); // 红色
            }
            valueItem->setForeground(QBrush(textColor));
        } else {
            // 如果表格项不存在，创建新的表格项
            qDebug() << "[DeviceMonitor] 表格项不存在，创建新项，参数:'" << param.name() << "' 值:" << displayValue;
            valueItem = new QTableWidgetItem(QString::number(displayValue, 'f', 1));
            parameterTable->setItem(row, 1, valueItem);
            
            // 设置文本颜色
            QColor textColor;
            if (isDisconnected) {
                // 未连接状态下文本颜色为红色
                textColor = QColor(239, 68, 68); // 红色
            } else if (param.isNormal()) {
                textColor = QColor(34, 197, 94); // 绿色
            } else {
                textColor = QColor(239, 68, 68); // 红色
            }
            valueItem->setForeground(QBrush(textColor));
            valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        }
        row++;
    }
}

QString DeviceMonitorPanel::getStatusStyle(DeviceStatus status)
{
    if (status == DeviceStatus::NORMAL) {
        return "background-color: #22c55e; color: white; border-radius: 4px; padding: 2px 6px;";
    } else if (status == DeviceStatus::DISCONNECTED) {
        return "background-color:rgb(167, 175, 12); color: white; border-radius: 4px; padding: 2px 6px;";
    } else {
        return "background-color: #ef4444; color: white; border-radius: 4px; padding: 2px 6px;";
    }
}

QString DeviceMonitorPanel::getStatusText(DeviceStatus status)
{
    if (status == DeviceStatus::NORMAL) {
        return "正常";
    } else if (status == DeviceStatus::DISCONNECTED) {
        return "未连接";
    } else {
        return "异常";
    }
}

QString DeviceMonitorPanel::getParameterValueStyle(const DeviceParameter &param)
{
    if (param.isNormal()) {
        return "color: #22c55e; font-weight: bold;";
    } else {
        return "color: #ef4444; font-weight: bold;";
    }
}