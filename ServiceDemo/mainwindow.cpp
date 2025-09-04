#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
//new
#include <QGroupBox>
#include <QPointer>
#include "devicedialog.h"
#include "ticketsdialog.h"
#include "deviceworker.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent, Qt::FramelessWindowHint)
    , ui(new Ui::MainWindow)
    , factoryServer(new QTcpServer(this))
    , expertServer(new QTcpServer(this))
{
    ui->setupUi(this); // 必须先调用这个

    // 设置UI和样式
    setupUI();

    setWindowTitle("服务端");

    // 创建空状态页面
    createEmptyStatePage();

    // 初始化数据库
    if (!initDatabase()) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法初始化数据库: " + db.lastError().text());
        return;
    }

    // 启动服务器
    if (!factoryServer->listen(QHostAddress::Any, 12345)) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法启动工厂服务器接口: " + factoryServer->errorString());
        return;
    } else {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 工厂服务器启动成功，监听端口: 12345");
    }

    if (!expertServer->listen(QHostAddress::Any, 12346)) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法启动专家服务器接口: " + expertServer->errorString());
        return;
    } else {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 专家服务器启动成功，监听端口: 12346");
    }

    connect(factoryServer, &QTcpServer::newConnection, this, &MainWindow::onFactoryNewConnection);
    connect(expertServer, &QTcpServer::newConnection, this, &MainWindow::onExpertNewConnection);

    // 设置默认窗口大小
    resize(1200, 800);

    // 避免主界面关闭时触发全局退出
    setAttribute(Qt::WA_QuitOnClose, false);

    // 设置无边框窗口 - 需要额外处理窗口拖动
    setWindowFlags(Qt::FramelessWindowHint);
}

MainWindow::~MainWindow()
{
    // 停止设备线程
    if (m_deviceRunning.loadAcquire() == 1 && m_deviceWorker) {
        QMetaObject::invokeMethod(m_deviceWorker, "stop", Qt::QueuedConnection);
        if (m_deviceThread) {
            m_deviceThread->quit();
            m_deviceThread->wait(1000); // 等待1秒
        }
    }
    db.close();
    delete ui;
}


void MainWindow::createEmptyStatePage()
{
    openFeatureInTab("home","主页");
}

void MainWindow::setupUI()
{
    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建左侧边栏
    QWidget *sidebarWidget = new QWidget(this);
    sidebarWidget->setMinimumWidth(200);
    sidebarWidget->setMaximumWidth(200);
    sidebarWidget->setStyleSheet("background-color: #1e293b;");
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setSpacing(0);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);

    // 保存侧边栏布局指针
    m_sidebarLayout = sidebarLayout;

    // 添加左侧边栏到主布局
    mainLayout->addWidget(sidebarWidget);

    // 创建右侧主内容区
    QWidget *mainContentWidget = new QWidget(this);
    QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContentWidget);
    mainContentLayout->setSpacing(0);
    mainContentLayout->setContentsMargins(0, 0, 0, 0);

    // 创建顶部栏
    ui->tabBar = new QTabBar(this);
    ui->tabBar->setStyleSheet(
        "QTabBar::tab { padding: 10px 20px; background-color: #f1f5f9; border-bottom: 1px solid #cbd5e1; }"
        "QTabBar::tab:selected { background-color: #ffffff; border-bottom: 2px solid #3b82f6; }"
        "QTabBar::close-button { image: url(:/icons/close.svg); subcontrol-position: right; }");
    ui->tabBar->setExpanding(false);
    ui->tabBar->setTabsClosable(true);
    ui->tabBar->setMovable(true);

    // 创建顶部栏右侧窗口控制按钮
    QWidget *topBarRightWidget = new QWidget(this);
    topBarRightWidget->setMaximumHeight(40);
    QHBoxLayout *topBarRightLayout = new QHBoxLayout(topBarRightWidget);
    topBarRightLayout->setContentsMargins(10, 5, 10, 5);

    // 添加最小化按钮
    QPushButton *minimizeButton = new QPushButton(this);
    minimizeButton->setIcon(QIcon(":/icons/minimize.svg"));
    minimizeButton->setIconSize(QSize(16, 16));
    minimizeButton->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; padding: 0px; margin: 0px; width: 30px; height: 30px; }"
        "QPushButton:hover { background-color: rgba(0, 0, 0, 0.1); }"
        );
    connect(minimizeButton, &QPushButton::clicked, this, [=]() {
        this->showMinimized();
    });

    // 添加窗口化按钮
    QPushButton *windowButton = new QPushButton(this);
    windowButton->setIconSize(QSize(16, 16));
    windowButton->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; padding: 0px; margin: 0px; width: 30px; height: 30px; }"
        "QPushButton:hover { background-color: rgba(0, 0, 0, 0.1); }"
        );

    // 初始图标设置为windowed.svg
    windowButton->setIcon(QIcon(":/icons/windowed.svg"));

    this->showMaximized();

    connect(windowButton, &QPushButton::clicked, this, [=]() {
        if (this->isMaximized()) {
            this->showNormal();
            windowButton->setIcon(QIcon(":/icons/maximize.svg"));
        } else {
            this->showMaximized();
            windowButton->setIcon(QIcon(":/icons/windowed.svg"));
        }
    });

    // 添加关闭按钮
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/icons/close.svg"));
    closeButton->setIconSize(QSize(16, 16));
    closeButton->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; padding: 0px; margin: 0px; width: 30px; height: 30px; }"
        "QPushButton:hover { background-color: rgba(239, 68, 68, 0.1); }"
        );
    connect(closeButton, &QPushButton::clicked, this, []() {
        QApplication::quit();
    });

    topBarRightLayout->addStretch();
    topBarRightLayout->addWidget(minimizeButton);
    topBarRightLayout->addWidget(windowButton);
    topBarRightLayout->addWidget(closeButton);

    // 创建顶部栏容器
    QWidget *topBarWidget = new QWidget(this);
    topBarWidget->setMaximumHeight(40);
    topBarWidget->setStyleSheet("background-color: #f1f5f9; border-bottom: 1px solid #cbd5e1;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(0);

    topBarLayout->addWidget(ui->tabBar);
    topBarLayout->addWidget(topBarRightWidget, 1);

    // 创建中央部件 - 使用分割布局
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #e2e8f0; }");

    // 左侧区域 (2/3) - 用于放置功能窗口
    QWidget *leftWidget = new QWidget(mainSplitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 创建堆叠窗口用于显示功能页面
    ui->stackedWidget = new QStackedWidget(leftWidget);
    ui->stackedWidget->setStyleSheet("background-color: #ffffff; border: 1px solid #cbd5e1;");
    leftLayout->addWidget(ui->stackedWidget);

    // 右侧区域 (1/3) - 用于显示服务器信息
    QWidget *rightWidget = new QWidget(mainSplitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 服务器信息标签
    QLabel *serverInfoLabel = new QLabel("服务器信息", rightWidget);
    serverInfoLabel->setStyleSheet("font-weight: bold; padding: 8px; background-color: #f8fafc; border-bottom: 1px solid #e2e8f0;");
    serverInfoLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(serverInfoLabel);

    // 服务器信息文本框
    ui->textEdit = new QTextEdit(rightWidget);
    ui->textEdit->setStyleSheet("background-color: #f1f5f9; border: none; font-family: 'Courier New'; font-size: 10pt;");
    ui->textEdit->setReadOnly(true);
    rightLayout->addWidget(ui->textEdit);

    // 设置分割比例 2:1
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 1);

    // 设置初始大小比例
    QList<int> sizes;
    sizes << width() * 2 / 3 << width() * 1 / 3;
    mainSplitter->setSizes(sizes);

    // 添加顶部栏和中央分割器到主内容布局
    mainContentLayout->addWidget(topBarWidget);
    mainContentLayout->addWidget(mainSplitter, 1);

    // 添加主内容区到主布局
    mainLayout->addWidget(mainContentWidget, 1);

    // 创建侧边栏
    createSidebar();

    // 连接信号和槽
    connect(ui->tabBar, &QTabBar::currentChanged, this, &MainWindow::on_tabBar_currentChanged);
    connect(ui->tabBar, &QTabBar::tabCloseRequested, this, &MainWindow::on_tabBar_tabCloseRequested);

    // 初始化默认页面 - 必须先创建主页
    openFeatureInTab("home", "主页");

    // 设置当前页面为主页
    if (m_featurePageMap.contains("home")) {
        int homeIndex = m_featurePageMap.value("home");
        ui->stackedWidget->setCurrentIndex(homeIndex);
        ui->tabBar->setCurrentIndex(0);
    }
}

void MainWindow::on_tabBar_currentChanged(int index)
{
    if (index < 0 || index >= m_tabFeatureNames.size()) return;

    QString featureName = m_tabFeatureNames[index];
    int pageIdx = m_featurePageMap.value(featureName, -1);

    qDebug() << "切换到标签:" << featureName << "页面索引:" << pageIdx;

    if (pageIdx >= 0 && pageIdx < ui->stackedWidget->count()) {
        ui->stackedWidget->setCurrentIndex(pageIdx);
    } else {
        // 页面无效，重新创建
        qDebug() << "页面无效，重新创建:" << featureName;
        openFeatureInTab(featureName, getTabNameFromFeature(featureName));
    }
}

// 关闭标签页
void MainWindow::on_tabBar_tabCloseRequested(int index)
{
    if (index < 0 || index >= m_tabFeatureNames.size()) return;

    QString feat = m_tabFeatureNames[index];

    // 主页不允许关闭
    if (feat == "home") {
        qDebug() << "主页不允许关闭";
        return;
    }

    qDebug() << "关闭标签页:" << feat;

    // 移除数据
    QString featureName = m_tabFeatureNames.takeAt(index);
    int pageIdx = m_featurePageMap.take(featureName);

    // 移除UI
    ui->tabBar->removeTab(index);
    QWidget* widget = ui->stackedWidget->widget(pageIdx);
    if (widget) {
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    // 更新其他页面索引
    for (auto it = m_featurePageMap.begin(); it != m_featurePageMap.end(); ++it) {
        if (it.value() > pageIdx) {
            it.value()--;
        }
    }

    // 如果没有标签页，创建空状态
    if (ui->tabBar->count() == 0) {
        createEmptyStatePage();
    }
}

// 打开或切换到功能标签页
void MainWindow::openFeatureInTab(const QString& featureName, const QString& tabName)
{
    qDebug() << "打开功能标签页:" << featureName;

    // 已存在 → 直接切换
    if (m_featurePageMap.contains(featureName)) {
        int tabIndex = m_tabFeatureNames.indexOf(featureName);
        if (tabIndex != -1) {
            ui->tabBar->setCurrentIndex(tabIndex);
            return;
        }
    }

    // 移除空状态页面（如果存在）
    if (m_featurePageMap.contains("empty")) {
        removeFeaturePage("empty");
    }

    // 创建对应的页面
    QWidget* page = nullptr;
    if (featureName == "home") {
        page = createHomePage();  // 创建主页
    } else if (featureName == "device_monitor") {
        page = new DeviceDialog(this);
    } else if (featureName == "tickets") {
        page = new TicketsDialog(this);
    } else if (featureName == "meeting") {
        page = new MeetingRoomDialog(this);
    } else {
        qWarning() << "未知的功能名称:" << featureName;
        return;
    }

    if (!page) return;

    // 设置页面属性
    page->setParent(ui->stackedWidget);

    // 添加到堆叠窗口
    int pageIndex = ui->stackedWidget->addWidget(page);
    m_featurePageMap.insert(featureName, pageIndex);
    m_tabFeatureNames.append(featureName);

    // 添加标签页
    int tabIndex = ui->tabBar->addTab(tabName);
    ui->tabBar->setCurrentIndex(tabIndex);

    qDebug() << "创建完成 - 功能:" << featureName << "页面索引:" << pageIndex << "标签索引:" << tabIndex;
}

// 创建主页
QWidget* MainWindow::createHomePage()
{
    qDebug() << "创建主页";

    QWidget *homePage = new QWidget();
    homePage->setObjectName("homePage");

    QGridLayout *gridLayout = new QGridLayout(homePage);
    gridLayout->setSpacing(20);
    gridLayout->setContentsMargins(40, 40, 40, 40);

    // 创建四个功能按钮
    QPushButton *buttons[] = {
        createHomeButton("设备监控", ":/icons/device.svg"),
        createHomeButton("启动设备", ":/icons/start.svg"),
        createHomeButton("工单列表", ":/icons/ticket.svg"),
        createHomeButton("会议管理", ":/icons/meeting.svg")
    };

    // 连接信号
    connect(buttons[0], &QPushButton::clicked, this, &MainWindow::on_pushButton1_clicked);
    connect(buttons[1], &QPushButton::clicked, this, &MainWindow::on_pushButton3_clicked);
    connect(buttons[2], &QPushButton::clicked, this, &MainWindow::on_pushButton2_clicked);
    connect(buttons[3], &QPushButton::clicked, this, &MainWindow::on_meetingButton_clicked);

    // 添加到布局
    for (int i = 0; i < 4; ++i) {
        buttons[i]->setParent(homePage);
        gridLayout->addWidget(buttons[i], i / 2, i % 2);
    }

    gridLayout->setRowStretch(2, 1);
    gridLayout->setColumnStretch(2, 1);

    return homePage;
}

// 辅助函数：根据功能名称获取标签名称
QString MainWindow::getTabNameFromFeature(const QString& featureName)
{
    static QMap<QString, QString> featureToName = {
        {"home", "主页"},
        {"device_monitor", "设备监控"},
        {"tickets", "工单列表"},
        {"meeting", "会议管理"}
    };

    return featureToName.value(featureName, "未知");
}

// 辅助函数：移除功能页面
void MainWindow::removeFeaturePage(const QString& featureName)
{
    if (m_featurePageMap.contains(featureName)) {
        int pageIdx = m_featurePageMap.value(featureName);
        int tabIdx = m_tabFeatureNames.indexOf(featureName);

        // 移除UI
        if (tabIdx != -1) {
            ui->tabBar->removeTab(tabIdx);
        }

        QWidget* widget = ui->stackedWidget->widget(pageIdx);
        if (widget) {
            ui->stackedWidget->removeWidget(widget);
            delete widget;
        }

        // 移除数据
        m_featurePageMap.remove(featureName);
        m_tabFeatureNames.removeAll(featureName);
    }
}

// 修改各个按钮点击处理函数
void MainWindow::on_pushButton1_clicked()
{
    openFeatureInTab("device_monitor", "设备监控");
}

void MainWindow::on_pushButton2_clicked()
{
    openFeatureInTab("tickets", "工单列表");
}

void MainWindow::on_meetingButton_clicked()
{
    openFeatureInTab("meeting", "会议管理");
}

void MainWindow::on_pushButton3_clicked()
{
    static bool running = false;
    qDebug() << "=== 点击启动设备按钮 ===";
    qDebug() << "当前运行状态:" << running;

    if (!running) {
        qDebug() << "开始启动设备监控...";

        qDebug() << "创建DeviceWorker和QThread对象...";
        m_deviceWorker = new DeviceWorker;
        m_deviceThread = new QThread(this);

        qDebug() << "将worker移动到线程...";
        m_deviceWorker->moveToThread(m_deviceThread);

        qDebug() << "连接信号和槽...";
        // 修改连接方式：使用QueuedConnection确保线程安全
        connect(m_deviceThread, &QThread::started, m_deviceWorker, &DeviceWorker::start);
        connect(m_deviceWorker, &DeviceWorker::finished, m_deviceThread, &QThread::quit);
        connect(m_deviceWorker, &DeviceWorker::finished, m_deviceWorker, &DeviceWorker::deleteLater);
        connect(m_deviceThread, &QThread::finished, m_deviceThread, &QThread::deleteLater);



        // 只连接数据信号，不包含数据库操作
        connect(m_deviceWorker, &DeviceWorker::newData, this, [this](const DeviceParams &p){
            qDebug() << "=== 收到新设备数据 ===";
            qDebug() << "数据线程:" << QThread::currentThreadId();
            qDebug() << "温度:" << p.temperature << "压力:" << p.pressure << "振动:" << p.vibration;
            qDebug() << "状态:" << p.status << "时间:" << p.lastUpdate.toString();

            broadcastDeviceParams(p);
            qDebug() << "已广播设备参数";

            if (!activeOrderId_.isEmpty()) {
                qDebug() << "有活跃工单:" << activeOrderId_ << "，开始处理数据...";

                // 实时推送（专家订阅）- 这是线程安全的
                QJsonObject obj;
                obj["ts"] = p.lastUpdate.toMSecsSinceEpoch();
                obj["temperature"] = p.temperature;
                obj["pressure"] = p.pressure;
                obj["vibration"] = p.vibration;
                obj["current"] = p.current;
                obj["voltage"] = p.voltage;
                obj["speed"] = p.speed;
                obj["status"] = p.status ? 1 : 0;

                QJsonArray logs;
                logs.append(QString("T=%1,P=%2,V=%3")
                                .arg(p.temperature,0,'f',1)
                                .arg(p.pressure,0,'f',1)
                                .arg(p.vibration,0,'f',2));
                obj["logs"] = logs;

                broadcastTelemetry(activeOrderId_, obj);
                qDebug() << "已广播遥测数据";

                // 将数据库操作移到DeviceWorker中执行
                QMetaObject::invokeMethod(m_deviceWorker, "saveDeviceDataWithTicket",
                                          Qt::QueuedConnection,
                                          Q_ARG(DeviceParams, p),
                                          Q_ARG(QString, activeOrderId_));

                qDebug() << "已请求保存设备数据到数据库";
            } else {
                qDebug() << "无活跃工单，跳过数据库操作";

                // 保存设备数据但不关联工单
                QMetaObject::invokeMethod(m_deviceWorker, "saveDeviceData",
                                          Qt::QueuedConnection,
                                          Q_ARG(DeviceParams, p));
            }

            qDebug() << "=== 设备数据处理完成 ===";
        }, Qt::QueuedConnection); // 确保使用队列连接

        qDebug() << "启动设备线程...";
        m_deviceThread->start();

        // 正确的检查位置：在start()之后
        qDebug() << "m_deviceThread is null?" << (m_deviceThread == nullptr);
        qDebug() << "m_deviceWorker is null?" << (m_deviceWorker == nullptr);
        qDebug() << "线程运行状态:" << m_deviceThread->isRunning();

        //ui->pushButton3->setText("停止设备");
        qDebug() << "按钮文本更新为: 停止设备";

        // 更新主页按钮状态
        updateHomeButtonState("启动设备", true);
        qDebug() << "主页按钮状态更新";

        running = true;
        qDebug() << "运行状态设置为: true";

        // 在服务器信息区域显示启动消息
        QString startMsg = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - 设备监控已启动";
        ui->textEdit->append(startMsg);
        qDebug() << "服务器信息更新:" << startMsg;

        qDebug() << "=== 设备启动完成 ===";

    } else {
        qDebug() << "开始停止设备监控...";

        if (m_deviceWorker) {
            qDebug() << "调用DeviceWorker的stop方法...";
            QMetaObject::invokeMethod(m_deviceWorker, "stop", Qt::QueuedConnection);
        } else {
            qDebug() << "m_deviceWorker为nullptr";
        }

        qDebug() << "按钮文本更新为: 启动设备";

        // 更新主页按钮状态
        updateHomeButtonState("启动设备", false);
        qDebug() << "主页按钮状态更新";

        running = false;
        qDebug() << "运行状态设置为: false";

        // 在服务器信息区域显示停止消息
        QString stopMsg = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - 设备监控已停止";
        ui->textEdit->append(stopMsg);
        qDebug() << "服务器信息更新:" << stopMsg;

        qDebug() << "=== 设备停止完成 ===";
    }

    qDebug() << "=== 按钮点击处理完成 ===";
}
QPushButton* MainWindow::createHomeButton(const QString &text, const QString &iconPath)
{
    QPushButton *button = new QPushButton();

    // 设置按钮文本和图标
    button->setText(text);

    // 设置图标（如果存在）
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        QIcon icon(iconPath);
        if (!icon.isNull()) {
            button->setIcon(icon);
            button->setIconSize(QSize(50, 50)); // 适中大小的图标
        } else {
            qDebug() << "图标加载失败:" << iconPath;
        }
    }

    // 设置按钮样式
    button->setStyleSheet(R"(
        QPushButton {
            background-color: #ffffff;
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            padding: 20px;
            font-size: 14px;
            font-weight: bold;
            color: #334155;
            min-width: 140px;
            min-height: 120px;
            text-align: center;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #1e40af;
        }
        QPushButton:pressed {
            background-color: #e2e8f0;
            border-color: #2563eb;
            color: #1e40af;
        }
        QPushButton:disabled {
            background-color: #f1f5f9;
            border-color: #cbd5e1;
            color: #94a3b8;
        }
    )");

    // 设置按钮大小策略
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    return button;
}

// 更新主页按钮状态
void MainWindow::updateHomeButtonState(const QString &buttonName, bool isRunning)
{
    // 查找主页
    if (!m_featurePageMap.contains("home")) {
        qDebug() << "主页不存在，无法更新按钮状态";
        return;
    }

    int homeIndex = m_featurePageMap.value("home");
    QWidget* homePage = ui->stackedWidget->widget(homeIndex);
    if (!homePage) {
        qDebug() << "主页部件无效，无法更新按钮状态";
        return;
    }

    // 查找指定名称的按钮
    QPushButton* targetButton = nullptr;
    QList<QPushButton*> buttons = homePage->findChildren<QPushButton*>();

    for (QPushButton* button : buttons) {
        if (button->text() == buttonName) {
            targetButton = button;
            break;
        }
    }

    if (!targetButton) {
        qDebug() << "未找到按钮:" << buttonName;
        return;
    }

    // 根据运行状态更新样式
    if (isRunning) {
        // 运行状态样式
        targetButton->setStyleSheet(R"(
            QPushButton {
                background-color: #dcfce7;
                border: 2px solid #22c55e;
                border-radius: 12px;
                padding: 20px;
                font-size: 14px;
                font-weight: bold;
                color: #166534;
                min-width: 140px;
                min-height: 120px;
                text-align: center;
            }
            QPushButton:hover {
                background-color: #bbf7d0;
                border-color: #16a34a;
                color: #14532d;
            }
            QPushButton:pressed {
                background-color: #a7f3d0;
                border-color: #15803d;
                color: #14532d;
            }
        )");

        // 可以添加运行状态的额外指示
        targetButton->setToolTip("正在运行 - 点击停止");

    } else {
        // 停止状态样式（恢复默认样式）
        targetButton->setStyleSheet(R"(
            QPushButton {
                background-color: #ffffff;
                border: 2px solid #e2e8f0;
                border-radius: 12px;
                padding: 20px;
                font-size: 14px;
                font-weight: bold;
                color: #334155;
                min-width: 140px;
                min-height: 120px;
                text-align: center;
            }
            QPushButton:hover {
                background-color: #f8fafc;
                border-color: #3b82f6;
                color: #1e40af;
            }
            QPushButton:pressed {
                background-color: #e2e8f0;
                border-color: #2563eb;
                color: #1e40af;
            }
        )");

        targetButton->setToolTip("点击启动");
    }

    // 更新按钮文本（可选）

    qDebug() << "更新按钮状态:" << buttonName << "运行状态:" << isRunning;
}
void MainWindow::createSidebar()
{
    // 使用成员变量 m_sidebarLayout
    QVBoxLayout *sidebarLayout = m_sidebarLayout;
    if (!sidebarLayout) {
        qWarning() << "Sidebar layout is null!";
        return;
    }

    // 清空现有布局内容（如果有）
    QLayoutItem* item;
    while ((item = sidebarLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // ----------- 设备组 -----------
    QGroupBox *deviceGroup = new QGroupBox("设备", this);
    deviceGroup->setStyleSheet(
        "QGroupBox { color:#e2e8f0; font-weight:bold; margin-top:10px; background-color: #1e293b; border: 1px solid #334155; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; color: #e2e8f0; }");
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceGroup);
    deviceLayout->setContentsMargins(10, 25, 10, 10);
    deviceLayout->setSpacing(8);

    QPushButton *deviceBtn = new QPushButton("设备监控", this);
    deviceBtn->setObjectName("PushButton1");
    deviceBtn->setStyleSheet(
        "QPushButton { background-color: #475569; color: white; border: none; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3b82f6; }"
        "QPushButton:pressed { background-color: #2563eb; }");
    connect(deviceBtn, &QPushButton::clicked, this, &MainWindow::on_pushButton1_clicked);
    deviceLayout->addWidget(deviceBtn);

    QPushButton *startBtn = new QPushButton("启动设备", this);
    startBtn->setObjectName("PushButton3");
    startBtn->setStyleSheet(
        "QPushButton { background-color: #475569; color: white; border: none; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3b82f6; }"
        "QPushButton:pressed { background-color: #2563eb; }");
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::on_pushButton3_clicked);
    deviceLayout->addWidget(startBtn);
    sidebarLayout->addWidget(deviceGroup);

    // ----------- 工单组 -----------
    QGroupBox *workGroup = new QGroupBox("工单", this);
    workGroup->setStyleSheet(
        "QGroupBox { color:#e2e8f0; font-weight:bold; margin-top:10px; background-color: #1e293b; border: 1px solid #334155; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; color: #e2e8f0; }");
    QVBoxLayout *workLayout = new QVBoxLayout(workGroup);
    workLayout->setContentsMargins(10, 25, 10, 10);
    workLayout->setSpacing(8);

    QPushButton *listBtn = new QPushButton("工单列表", this);
    listBtn->setObjectName("PushButton2");
    listBtn->setStyleSheet(
        "QPushButton { background-color: #475569; color: white; border: none; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3b82f6; }"
        "QPushButton:pressed { background-color: #2563eb; }");
    connect(listBtn, &QPushButton::clicked, this, &MainWindow::on_pushButton2_clicked);
    workLayout->addWidget(listBtn);
    sidebarLayout->addWidget(workGroup);

    // ----------- 会议组 -----------
    QGroupBox *meetGroup = new QGroupBox("会议", this);
    meetGroup->setStyleSheet(
        "QGroupBox { color:#e2e8f0; font-weight:bold; margin-top:10px; background-color: #1e293b; border: 1px solid #334155; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; color: #e2e8f0; }");
    QVBoxLayout *meetLayout = new QVBoxLayout(meetGroup);
    meetLayout->setContentsMargins(10, 25, 10, 10);
    meetLayout->setSpacing(8);

    QPushButton *meetBtn = new QPushButton("会议管理", this);
    meetBtn->setObjectName("meetingButton");
    meetBtn->setStyleSheet(
        "QPushButton { background-color: #475569; color: white; border: none; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3b82f6; }"
        "QPushButton:pressed { background-color: #2563eb; }");
    connect(meetBtn, &QPushButton::clicked, this, &MainWindow::on_meetingButton_clicked);
    meetLayout->addWidget(meetBtn);
    sidebarLayout->addWidget(meetGroup);

    // 添加弹性空间
    sidebarLayout->addStretch();

    qDebug() << "Sidebar created successfully with" << sidebarLayout->count() << "items";
}

bool MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("industrial.db");

    if (!db.open()) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 数据库打开错误: " + db.lastError().text());
        return false;
    }

    QSqlQuery query;
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS factory_users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS expert_users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS tickets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            priority TEXT NOT NULL,
            status INTEGER DEFAULT 0,
            creator TEXT NOT NULL,
            expert_username TEXT,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            solution TEXT,
            device_params TEXT
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS device_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temperature REAL, pressure REAL, vibration REAL,
            current REAL, voltage REAL, speed INTEGER,
            status BOOLEAN,
            record_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    // 新增：索引与“简化知识库”关联表/故障表
    query.exec("CREATE INDEX IF NOT EXISTS idx_device_history_time ON device_history(record_time)");

    query.exec(R"(
      CREATE TABLE IF NOT EXISTS faults (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ticket_id INTEGER,
        code TEXT, text TEXT, level TEXT,
        ts DATETIME DEFAULT CURRENT_TIMESTAMP
    ))");
    query.exec("CREATE INDEX IF NOT EXISTS idx_faults_ticket ON faults(ticket_id)");

    query.exec(R"(
      CREATE TABLE IF NOT EXISTS ticket_device_history (
        ticket_id INTEGER NOT NULL,
        history_id INTEGER NOT NULL,
        PRIMARY KEY(ticket_id, history_id)6
    ))");

    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - 数据库初始化完成");
    return true;
}

void MainWindow::showTicketsDialog()
{
    TicketsDialog dlg(this);
    dlg.exec();
}

void MainWindow::onFactoryNewConnection()
{
    QTcpSocket *socket = factoryServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    clientTypeMap.insert(socket, "FACTORY");

    const QString clientInfo = QString("工厂客户端连接: %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + clientInfo);

    sendResponse(socket, "TYPE|FACTORY|ACK");
}

void MainWindow::onExpertNewConnection()
{
    QTcpSocket *socket = expertServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    clientTypeMap.insert(socket, "EXPERT");

    expertSockets.insert(socket);
    connect(socket, &QTcpSocket::disconnected, this, [=]{
        expertSockets.remove(socket);
        const QString oid = expertJoinedOrder.take(socket);
        if (!oid.isEmpty()) {
            orderExperts[oid].remove(socket);
            teleSubs[oid].remove(socket);
        }
    });

    const QString clientInfo = QString("专家客户端连接: %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + clientInfo);

    sendResponse(socket, "TYPE|EXPERT|ACK");
}

void MainWindow::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        const QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (line.isEmpty()) continue;

        const QString clientType = clientTypeMap.value(socket);
        const QString clientInfo = QString("%1客户端 %2:%3")
                                       .arg(clientType)
                                       .arg(socket->peerAddress().toString())
                                       .arg(socket->peerPort());
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                             + " - 收到来自 " + clientInfo + " 的数据: " + line);
        processLine(socket, line);
    }
}

void MainWindow::processLine(QTcpSocket* socket, const QString& data)
{
    const QStringList parts = data.split('|');
    if (parts.isEmpty()) return;

    const QString action = parts[0];
    const QString clientType = clientTypeMap.value(socket);

    if (action == "LOGIN") {
        if (parts.size() >= 3) handleLogin(socket, parts[1], parts[2], clientType);
        return;
    }

    if (action == "REGISTER") {
        if (parts.size() >= 3) handleRegister(socket, parts[1], parts[2], clientType);
        return;
    }

    if (action == "CREATE_TICKET") {
        handleCreateTicket(socket, parts);
        return;
    }

    if (action == "ORDER") {
        const QString sub = parts.value(1);
        if (sub == "JOIN") {
            const QString oid = parts.value(2);
            
            // 检查工单是否存在
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT id FROM tickets WHERE id = :id");
            checkQuery.bindValue(":id", oid.toInt());
            if (!checkQuery.exec() || !checkQuery.next()) {
                // 工单不存在
                sendResponse(socket, QString("ORDER|JOIN|ERR|工单 %1 不存在，无法加入").arg(oid));
                return;
            }
            
            // 工单存在，继续加入逻辑
            if (clientType == "EXPERT") {
                orderExperts[oid].insert(socket);
                expertJoinedOrder[socket] = oid;
            } else if (clientType == "FACTORY") {
                orderFactories[oid].insert(socket);
                factoryJoinedOrder[socket] = oid;
            }
            activeOrderId_ = oid;
            sendResponse(socket, QString("ORDER|JOIN|OK|%1").arg(oid));
            return;
        }
        if (sub == "LEAVE") {
            const QString oid = parts.value(2);
            if (clientType == "EXPERT") {
                orderExperts[oid].remove(socket);
                teleSubs[oid].remove(socket);
                expertJoinedOrder.remove(socket);
            } else if (clientType == "FACTORY") {
                orderFactories[oid].remove(socket);
                factoryJoinedOrder.remove(socket);
            }
            if (activeOrderId_ == oid) activeOrderId_.clear();
            sendResponse(socket, QString("ORDER|LEAVE|OK|%1").arg(oid));
            return;
        }
        return;
    }

    if (action == "TELE") {
        const QString sub = parts.value(1);
        const QString oid = parts.value(2);
        if (sub == "SUB")   { teleSubs[oid].insert(socket);  return; }
        if (sub == "UNSUB") { teleSubs[oid].remove(socket);  return; }
        return;
    }

    if (action == "CHAT") {
        const QString sub = parts.value(1);
        if (sub == "SEND") {
            int p0 = data.indexOf('|');
            int p1 = data.indexOf('|', p0 + 1);
            int p2 = data.indexOf('|', p1 + 1);
            int p3 = data.indexOf('|', p2 + 1);
            if (p0 < 0 || p1 < 0 || p2 < 0 || p3 < 0) {
                sendResponse(socket, "ERROR|CHAT|BAD_FORMAT");
                return;
            }
            const QString orderId = data.mid(p1 + 1, p2 - (p1 + 1));
            const QString from    = data.mid(p2 + 1, p3 - (p2 + 1));
            const QString text    = data.mid(p3 + 1);
            QString safe = text; safe.replace('\n', ' ').replace('\r', ' ');

            ui->textEdit->append(QString("CHAT SEND: oid=%1, from=%2, text=%3")
                                     .arg(orderId, from, safe.left(80)));

            broadcastChat(orderId, from, safe);
        }
        return;
    }

    // 新增：历史工单（简化知识库）
    if (action == "TICKETS") {
        const QString sub = parts.value(1);
        if (sub == "LIST") {
            bool ok1=false, ok2=false;
            int page = parts.value(2).toInt(&ok1);
            int size = parts.value(3).toInt(&ok2);
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=50;
            handleTicketsList(socket, page, size);
            return;
        }
        if (sub == "LOGS") {
            bool okId=false, ok1=false, ok2=false;
            int tid  = parts.value(2).toInt(&okId);
            int page = parts.value(3).toInt(&ok1);
            int size = parts.value(4).toInt(&ok2);
            if (!okId || tid<=0) { sendResponse(socket,"TICKETS|LOGS|ERR|bad id"); return; }
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=100;
            handleTicketLogs(socket, tid, page, size);
            return;
        }
        if (sub == "FAULTS") {
            bool okId=false, ok1=false, ok2=false;
            int tid  = parts.value(2).toInt(&okId);
            int page = parts.value(3).toInt(&ok1);
            int size = parts.value(4).toInt(&ok2);
            if (!okId || tid<=0) { sendResponse(socket,"TICKETS|FAULTS|ERR|bad id"); return; }
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=100;
            handleFaultsByTicket(socket, tid, page, size);
            return;
        }
        return;
    }

    sendResponse(socket, "ERROR|Unknown action");
}

void MainWindow::handleLogin(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType)
{
    QString tableName;
    if (clientType == "FACTORY")       tableName = "factory_users";
    else if (clientType == "EXPERT")   tableName = "expert_users";
    else { sendResponse(socket, "LOGIN|FAIL|Invalid client type"); return; }

    QSqlQuery query;
    query.prepare("SELECT password FROM " + tableName + " WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec()) { sendResponse(socket, "LOGIN|FAIL|DB error"); return; }

    if (query.next()) {
        if (query.value(0).toString() == password) {
            sendResponse(socket, "LOGIN|SUCCESS");
            m_socketToUsername[socket] = username;
            if (clientType == "FACTORY") {
                m_factorySockets[username] = socket;
                ui->textEdit->append("工厂客户端 " + username + " 已添加到 m_factorySockets");
            }
        } else sendResponse(socket, "LOGIN|FAIL|Invalid password");
    } else sendResponse(socket, "LOGIN|FAIL|User not found");
}

void MainWindow::handleRegister(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType)
{
    QString tableName;
    if (clientType == "FACTORY")       tableName = "factory_users";
    else if (clientType == "EXPERT")   tableName = "expert_users";
    else { sendResponse(socket, "REGISTER|FAIL|Invalid client type"); return; }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM " + tableName + " WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec()) { sendResponse(socket, "REGISTER|FAIL|DB error"); return; }
    if (checkQuery.next()) { sendResponse(socket, "REGISTER|FAIL|Username already exists"); return; }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO " + tableName + " (username, password) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    if (insertQuery.exec()) sendResponse(socket, "REGISTER|SUCCESS");
    else sendResponse(socket, "REGISTER|FAIL|DB error");
}

void MainWindow::handleCreateTicket(QTcpSocket *socket, const QStringList &parts)
{
    if (clientTypeMap.value(socket) != "FACTORY" || parts.size() < 4) {
        sendResponse(socket, "ERROR|Invalid CREATE_TICKET command format");
        return;
    }
    const QString creator = m_socketToUsername.value(socket);
    if (creator.isEmpty()) { sendResponse(socket, "ERROR|Not logged in"); return; }

    const QString title = parts.value(1);
    const QString description = parts.value(2);
    const QString priority = parts.value(3);

    QSqlQuery query;
    query.prepare("INSERT INTO tickets (title, description, priority, status, creator) "
                  "VALUES (:title, :desc, :priority, :status, :creator)");
    query.bindValue(":title", title);
    query.bindValue(":desc", description);
    query.bindValue(":priority", priority);
    query.bindValue(":status", PENDING);
    query.bindValue(":creator", creator);
    if (!query.exec()) {
        sendResponse(socket, "ERROR|Database error: " + query.lastError().text());
        return;
    }
    const int ticketId = query.lastInsertId().toInt();
    activeOrderId_ = QString::number(ticketId);

    orderFactories[activeOrderId_].insert(socket);
    factoryJoinedOrder[socket] = activeOrderId_;

    sendResponse(socket, "CREATE_TICKET|SUCCESS|" + QString::number(ticketId));
    const QString invite = QString("ORDER|INVITE|%1|%2").arg(activeOrderId_, title);
    broadcastToExperts(invite);
}

void MainWindow::sendResponse(QTcpSocket *socket, const QString &response)
{
    if (socket && socket->isOpen()) {
        socket->write(response.toUtf8());
        socket->write("\n");
        socket->flush();
    }
}

void MainWindow::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    const QString username  = m_socketToUsername.value(socket);
    const QString clientType= clientTypeMap.value(socket);

    if (clientType == "FACTORY" && !username.isEmpty()) {
        m_factorySockets.remove(username);
        ui->textEdit->append("工厂客户端 " + username + " 已从 m_factorySockets 中移除");
    }

    const QString eoid = expertJoinedOrder.take(socket);
    if (!eoid.isEmpty()) {
        orderExperts[eoid].remove(socket);
        teleSubs[eoid].remove(socket);
    }
    const QString foid = factoryJoinedOrder.take(socket);
    if (!foid.isEmpty()) {
        orderFactories[foid].remove(socket);
    }

    m_socketToUsername.remove(socket);
    clientTypeMap.remove(socket);
    ui->textEdit->append("一个客户端已断开连接。");
}


void MainWindow::broadcastDeviceParams(const DeviceParams &params)
{
    const QString orderId = "ORDER_001";
    const qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    const QString jsonData = QString("{\"ts\":%1,\"temperature\":%2,\"pressure\":%3,"
                                     "\"vibration\":%4,\"current\":%5,\"voltage\":%6,"
                                     "\"speed\":%7,\"status\":%8}")
                                 .arg(timestamp)
                                 .arg(params.temperature, 0, 'f', 1)
                                 .arg(params.pressure,    0, 'f', 1)
                                 .arg(params.vibration,   0, 'f', 1)
                                 .arg(params.current,     0, 'f', 1)
                                 .arg(params.voltage,     0, 'f', 1)
                                 .arg(params.speed)
                                 .arg(params.status ? 1 : 0);

    const QString message = QString("TELE|DATA|%1|%2").arg(orderId, jsonData);
    for (QTcpSocket* s : m_factorySockets.values()) {
        if (s && s->isOpen()) sendResponse(s, message);
    }
}

void MainWindow::sendToExpert(QTcpSocket* s, const QString& line)
{
    if (s && s->isOpen()) {
        s->write(line.toUtf8());
        s->write("\n");
        s->flush();
    }
}

void MainWindow::broadcastToExperts(const QString& line)
{
    ui->textEdit->append("向所有在线专家广播: " + line);
    for (auto* s : expertSockets) sendToExpert(s, line);
}

void MainWindow::broadcastTelemetry(const QString& orderId, const QJsonObject& obj)
{
    if (!teleSubs.contains(orderId)) return;
    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    const QString line = QString("TELE|DATA|%1|%2").arg(orderId, json);
    for (auto* s : teleSubs[orderId]) sendToExpert(s, line);
}

void MainWindow::broadcastChat(const QString& orderId, const QString& from, const QString& text)
{
    const qint64 ts = QDateTime::currentMSecsSinceEpoch();
    const QString msg = QString("CHAT|MSG|%1|%2|%3|%4")
                            .arg(orderId, from, QString::number(ts), text);

    ui->textEdit->append(QString("向工单 %1 广播聊天：%2").arg(orderId, text.left(80)));

    for (QTcpSocket* s : orderExperts[orderId])   sendToExpert(s, msg);
    for (QTcpSocket* s : orderFactories[orderId]) sendToExpert(s, msg);
}

// 新增：历史工单查询实现
void MainWindow::handleTicketsList(QTcpSocket* socket, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;

    QSqlQuery q;
    QString sql = QString(
                      "SELECT id, title, description, priority, status, creator, "
                      "       IFNULL(expert_username,'') AS expert_username, created_time "
                      "FROM tickets ORDER BY created_time DESC LIMIT %1 OFFSET %2")
                      .arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|LIST|ERR|" + q.lastError().text()); return; }

    QSqlQuery qCountLog, qCountFault;
    qCountLog.prepare("SELECT COUNT(*) FROM ticket_device_history WHERE ticket_id=:tid");
    qCountFault.prepare("SELECT COUNT(*) FROM faults WHERE ticket_id=:tid");

    QJsonArray arr;
    while (q.next()) {
        const int tid = q.value("id").toInt();

        qCountLog.bindValue(":tid", tid); qCountLog.exec(); qCountLog.next();
        const int logsCount = qCountLog.value(0).toInt();

        qCountFault.bindValue(":tid", tid); qCountFault.exec(); qCountFault.next();
        const int faultsCount = qCountFault.value(0).toInt();

        QJsonObject o;
        o["id"]             = tid;
        o["title"]          = q.value("title").toString();
        o["description"]    = q.value("description").toString();
        o["priority"]       = q.value("priority").toString();
        o["status"]         = q.value("status").toInt();
        o["creator"]        = q.value("creator").toString();
        o["expert_username"]= q.value("expert_username").toString();
        o["created_time"]   = q.value("created_time").toString();
        o["logs"]           = logsCount;
        o["faults"]         = faultsCount;
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|LIST|OK|" + payload);
}

void MainWindow::handleTicketLogs(QTcpSocket* socket, int ticketId, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;
    QSqlQuery q;
    QString sql = QString(
                      "SELECT dh.id, dh.temperature, dh.pressure, dh.vibration, dh.current, dh.voltage, dh.speed, dh.status, dh.record_time "
                      "FROM ticket_device_history tdh "
                      "JOIN device_history dh ON dh.id = tdh.history_id "
                      "WHERE tdh.ticket_id=%1 "
                      "ORDER BY dh.record_time DESC LIMIT %2 OFFSET %3")
                      .arg(ticketId).arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|LOGS|ERR|" + q.lastError().text()); return; }

    QJsonArray arr;
    while (q.next()) {
        QJsonObject o;
        o["id"]          = q.value("id").toInt();
        o["temperature"] = q.value("temperature").toDouble();
        o["pressure"]    = q.value("pressure").toDouble();
        o["vibration"]   = q.value("vibration").toDouble();
        o["current"]     = q.value("current").toDouble();
        o["voltage"]     = q.value("voltage").toDouble();
        o["speed"]       = q.value("speed").toInt();
        o["status"]      = q.value("status").toBool() ? 1 : 0;
        o["record_time"] = q.value("record_time").toString();
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|LOGS|OK|" + payload);
}

void MainWindow::handleFaultsByTicket(QTcpSocket* socket, int ticketId, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;
    QSqlQuery q;
    QString sql = QString(
                      "SELECT id, code, text, level, ts FROM faults "
                      "WHERE ticket_id=%1 ORDER BY ts DESC LIMIT %2 OFFSET %3")
                      .arg(ticketId).arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|FAULTS|ERR|" + q.lastError().text()); return; }

    QJsonArray arr;
    while (q.next()) {
        QJsonObject o;
        o["id"]   = q.value("id").toInt();
        o["code"] = q.value("code").toString();
        o["text"] = q.value("text").toString();
        o["level"]= q.value("level").toString();
        o["ts"]   = q.value("ts").toString();
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|FAULTS|OK|" + payload);
}
