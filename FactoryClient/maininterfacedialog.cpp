#include "maininterfacedialog.h"
#include "ui_maininterfacedialog.h"
#include "mainwindow.h"
#include "registerdialog.h"
#include "devicemonitorpanel.h"
#include "workorderdialog.h"
#include "accountinfodialog.h"
#include "chatdialog.h"
#include "meetingdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabBar>
#include <QStackedWidget>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QApplication>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
MainInterfaceDialog::MainInterfaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainInterfaceDialog)
    , meetingDialog(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("工厂客户端 - 主界面");
    resize(1200, 800); // 设置默认窗口大小

    // 避免主界面关闭时触发全局退出
    setAttribute(Qt::WA_QuitOnClose, false);

    // 设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);

    // 设置UI
    setupUI();

    // 创建空状态页面
    createEmptyStatePage();
}

MainInterfaceDialog::~MainInterfaceDialog()
{
    delete ui;
}

void MainInterfaceDialog::setupUI()
{
    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
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

    // 由于无边框窗口的isMaximized()可能不准确，这里强制设置初始图标为windowed.svg
    // 因为程序默认是全屏状态
    windowButton->setIcon(QIcon(":/icons/windowed.svg"));

    // 确保程序启动时是全屏状态
    this->showMaximized();

    connect(windowButton, &QPushButton::clicked, this, [=]() {
        if (this->isMaximized()) {
            this->showNormal();
            // 切换到全屏图标
            windowButton->setIcon(QIcon(":/icons/maximize.svg"));
        } else {
            this->showMaximized();
            // 切换到窗口化图标
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
        QApplication::quit(); // 退出程序
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

    // 创建中央部件
    ui->stackedWidget = new QStackedWidget(this);
    ui->stackedWidget->setStyleSheet("background-color: #ffffff;");

    // 添加顶部栏和中央部件到主内容布局
    mainContentLayout->addWidget(topBarWidget);
    mainContentLayout->addWidget(ui->stackedWidget, 1);

    // 添加主内容区到主布局
    mainLayout->addWidget(mainContentWidget, 1);

    // 创建侧边栏
    createSidebar();

    // 连接信号和槽
    connect(ui->tabBar, &QTabBar::currentChanged, this, &MainInterfaceDialog::on_tabBar_currentChanged);
    connect(ui->tabBar, &QTabBar::tabCloseRequested, this, &MainInterfaceDialog::on_tabBar_tabCloseRequested);
}

void MainInterfaceDialog::createSidebar()
{
    // 【改】使用成员变量 m_sidebarLayout
    QVBoxLayout *sidebarLayout = m_sidebarLayout;
    if (!sidebarLayout) {
        return;

    }

    // 用户管理组
    QGroupBox *userGroupBox = new QGroupBox("用户管理", this);
    userGroupBox->setStyleSheet("QGroupBox { color: #e2e8f0; font-weight: bold; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *userLayout = new QVBoxLayout(userGroupBox);
    userLayout->setContentsMargins(10, 20, 10, 10);

    ui->loginButton = new QPushButton("账号信息", this);
    ui->loginButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->loginButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_loginButton_clicked);
    userLayout->addWidget(ui->loginButton);

    // 工单管理组
    QGroupBox *workOrderGroupBox = new QGroupBox("工单管理", this);
    workOrderGroupBox->setStyleSheet("QGroupBox { color: #e2e8f0; font-weight: bold; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *workOrderLayout = new QVBoxLayout(workOrderGroupBox);
    workOrderLayout->setContentsMargins(10, 20, 10, 10);

    ui->newWorkOrderButton = new QPushButton("新建工单", this);
    ui->newWorkOrderButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->newWorkOrderButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_newWorkOrderButton_clicked);
    workOrderLayout->addWidget(ui->newWorkOrderButton);

    ui->workOrderListButton = new QPushButton("历史工单", this);
    ui->workOrderListButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->workOrderListButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_workOrderListButton_clicked);
    workOrderLayout->addWidget(ui->workOrderListButton);

    // 通信功能组
    QGroupBox *communicationGroupBox = new QGroupBox("通信功能", this);
    communicationGroupBox->setStyleSheet("QGroupBox { color: #e2e8f0; font-weight: bold; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *communicationLayout = new QVBoxLayout(communicationGroupBox);
    communicationLayout->setContentsMargins(10, 20, 10, 10);

    ui->textMessageButton = new QPushButton("联系专家", this);
    ui->textMessageButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->textMessageButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_textMessageButton_clicked);
    communicationLayout->addWidget(ui->textMessageButton);


    ui->joinMeetingButton = new QPushButton("在线会议", this);
    ui->joinMeetingButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->joinMeetingButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_joinMeetingButton_clicked);
    communicationLayout->addWidget(ui->joinMeetingButton);


    // 设备监控组
    QGroupBox *deviceMonitorGroupBox = new QGroupBox("设备监控", this);
    deviceMonitorGroupBox->setStyleSheet("QGroupBox { color: #e2e8f0; font-weight: bold; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *deviceMonitorLayout = new QVBoxLayout(deviceMonitorGroupBox);
    deviceMonitorLayout->setContentsMargins(10, 20, 10, 10);

    ui->realTimeDataButton = new QPushButton("实时数据", this);
    ui->realTimeDataButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->realTimeDataButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_realTimeDataButton_clicked);
    deviceMonitorLayout->addWidget(ui->realTimeDataButton);

    ui->deviceRecordingButton = new QPushButton("设备录制", this);
    ui->deviceRecordingButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->deviceRecordingButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_deviceRecordingButton_clicked);
    deviceMonitorLayout->addWidget(ui->deviceRecordingButton);



    // 知识库组
    QGroupBox *knowledgeBaseGroupBox = new QGroupBox("知识库", this);
    knowledgeBaseGroupBox->setStyleSheet("QGroupBox { color: #e2e8f0; font-weight: bold; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; }");
    QVBoxLayout *knowledgeBaseLayout = new QVBoxLayout(knowledgeBaseGroupBox);
    knowledgeBaseLayout->setContentsMargins(10, 20, 10, 10);

    ui->knowledgeBaseButton = new QPushButton("企业知识库", this);
    ui->knowledgeBaseButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #e2e8f0; text-align: left; padding: 10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #334155; }"
        "QPushButton:pressed { background-color: #475569; }"
        );
    connect(ui->knowledgeBaseButton, &QPushButton::clicked, this, &MainInterfaceDialog::on_knowledgeBaseButton_clicked);
    knowledgeBaseLayout->addWidget(ui->knowledgeBaseButton);

    // 添加所有组到侧边栏布局
    sidebarLayout->addWidget(userGroupBox);
    sidebarLayout->addWidget(workOrderGroupBox);
    sidebarLayout->addWidget(communicationGroupBox);
    sidebarLayout->addWidget(deviceMonitorGroupBox);
    sidebarLayout->addWidget(knowledgeBaseGroupBox);
    sidebarLayout->addStretch();
}

void MainInterfaceDialog::createEmptyStatePage()
{
    QWidget *emptyPage = new QWidget(this);
    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyPage);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *emptyLabel = new QLabel("无工作任务正在进行", this);
    emptyLabel->setStyleSheet("font-size: 18px; color: #94a3b8;");
    emptyLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(emptyLabel);

    ui->stackedWidget->addWidget(emptyPage);
    m_featurePageMap.insert("empty", 0);
    m_tabFeatureNames.append("empty");
}

void MainInterfaceDialog::createAndShowFeaturePage(const QString &featureName, const QString &displayName)
{
    // 检查该功能是否已经打开
    int existingTabIndex = -1;
    for (int i = 0; i < m_tabFeatureNames.size(); i++) {
        if (m_tabFeatureNames[i] == featureName) {
            existingTabIndex = i;
            break;
        }
    }

    // 如果已经打开，则切换到该标签
    if (existingTabIndex >= 0) {
        ui->tabBar->setCurrentIndex(existingTabIndex);
        return;
    }

    // 如果是空状态页面，则移除它
    if (m_tabFeatureNames.size() == 1 && m_tabFeatureNames[0] == "empty") {
        ui->tabBar->removeTab(0);
        QWidget *emptyPage = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(emptyPage);
        delete emptyPage;
        m_featurePageMap.remove("empty");
        m_tabFeatureNames.removeAt(0);
    }

    // 创建功能内容部件
    QWidget *featureContentWidget = createFeatureContentWidget(featureName, displayName);

    // 添加到stackedWidget
    int pageIndex = ui->stackedWidget->addWidget(featureContentWidget);

    // 处理重复的显示名称
    QString uniqueDisplayName = displayName;
    int counter = 1;
    while (true) {
        bool found = false;
        for (int i = 0; i < ui->tabBar->count(); i++) {
            if (ui->tabBar->tabText(i) == uniqueDisplayName) {
                found = true;
                uniqueDisplayName = QString("%1 (%2)").arg(displayName).arg(counter++);
                break;
            }
        }
        if (!found) {
            break;
        }
    }

    // 添加标签
    int tabIndex = ui->tabBar->addTab(uniqueDisplayName);

    // 更新映射关系
    m_featurePageMap.insert(featureName, pageIndex);
    m_tabFeatureNames.append(featureName);

    // 切换到新标签
    ui->tabBar->setCurrentIndex(tabIndex);
}

QWidget *MainInterfaceDialog::createFeatureContentWidget(const QString &featureName, const QString &displayName)
{
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    // 根据功能名称设置内容
    if (featureName == "login") {
        AccountInfoDialog *accountWidget = new AccountInfoDialog;
        accountWidget->setUsername(m_username);
        
        // 连接退出登录信号
        connect(accountWidget, &AccountInfoDialog::logoutRequested, this, [this]() {
            // 1) 临时关闭"最后一个窗口关闭就退出"
            const bool oldQuit = qApp->quitOnLastWindowClosed();
            qApp->setQuitOnLastWindowClosed(false);

            // 2) 先创建并显示登录窗口（一定要是顶层：parent = nullptr）
            MainWindow* loginWindow = new MainWindow(nullptr);
            loginWindow->setAttribute(Qt::WA_DeleteOnClose);
            loginWindow->show();
            loginWindow->raise();
            loginWindow->activateWindow();

            // 3) 下一轮事件循环里再关闭当前主界面
            QTimer::singleShot(0, this, [this, oldQuit]{
                this->close(); // 或 this->deleteLater();
                // 4) 恢复原来的 quitOnLastWindowClosed
                qApp->setQuitOnLastWindowClosed(oldQuit);
            });
        });
        
        // 其他信号暂时不实现具体功能
        connect(accountWidget, &AccountInfoDialog::profileEditRequested, this, []() {
            // 个人资料编辑功能暂未实现
        });
        
        connect(accountWidget, &AccountInfoDialog::passwordChangeRequested, this, []() {
            // 修改密码功能暂未实现
        });

        contentLayout->addWidget(accountWidget, 1);
    } else if (featureName == "realTimeData") {
        // 实时设备数据监控 - 使用设备监控面板
        DeviceMonitorPanel *monitorPanel = new DeviceMonitorPanel(this);
        m_deviceMonitorPanel = monitorPanel; // 保存引用
        contentLayout->addWidget(monitorPanel, 1); // 添加拉伸因子，使其占满剩余空间

        // 应用之前缓存的设备数据
        applyDeviceDataCache();

        // 不添加addStretch，让DeviceMonitorPanel完全占满
    }
    else if (featureName == "joinMeeting") {
        // 加入会议页面 - 创建会议功能界面
        QLabel *titleLabel = new QLabel("远程会议功能", this);
        titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; margin-bottom: 20px;");
        contentLayout->addWidget(titleLabel);

        // 会议控制面板
        QWidget *meetingPanel = new QWidget(this);
        meetingPanel->setStyleSheet("background-color: #f8fafc; border-radius: 8px; padding: 20px;");
        QVBoxLayout *panelLayout = new QVBoxLayout(meetingPanel);

        // 会议状态显示
        QLabel *statusLabel = new QLabel("会议状态: 未连接", this);
        statusLabel->setStyleSheet("font-size: 14px; color: #475569; margin-bottom: 15px;");
        panelLayout->addWidget(statusLabel);

        // 按钮区域
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        // 创建会议按钮
        QPushButton *createMeetingBtn = new QPushButton("创建会议", this);
        createMeetingBtn->setStyleSheet(
            "QPushButton { background-color: #3b82f6; color: white; font-size: 14px; padding: 10px 20px; border-radius: 4px; border: none; }"
            "QPushButton:hover { background-color: #2563eb; }"
            "QPushButton:pressed { background-color: #1d4ed8; }"
            );

        // 打开独立会议窗口按钮
        QPushButton *openMeetingWindowBtn = new QPushButton("加入会议", this);
        openMeetingWindowBtn->setStyleSheet(
            "QPushButton { background-color: #8b5cf6; color: white; font-size: 14px; padding: 10px 20px; border-radius: 4px; border: none; }"
            "QPushButton:hover { background-color: #7c3aed; }"
            "QPushButton:pressed { background-color: #6d28d9; }"
            );

        // 连接打开独立窗口按钮
        connect(openMeetingWindowBtn, &QPushButton::clicked, this, [=]() {
            // 创建输入对话框
            QDialog *inputDialog = new QDialog(this);
            inputDialog->setWindowTitle("加入会议");
            inputDialog->setModal(true);
            inputDialog->resize(400, 200);

            QVBoxLayout *dialogLayout = new QVBoxLayout(inputDialog);

            // 标题
            QLabel *dialogTitle = new QLabel("请输入会议服务器信息", inputDialog);
            dialogTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 20px;");
            dialogLayout->addWidget(dialogTitle);

            // 服务器地址输入
            QHBoxLayout *serverLayout = new QHBoxLayout();
            QLabel *serverLabel = new QLabel("服务器地址:", inputDialog);
            QLineEdit *serverEdit = new QLineEdit(inputDialog);
            serverEdit->setPlaceholderText("例如: localhost 或 192.168.1.100");
            serverEdit->setText("localhost"); // 默认值
            serverLayout->addWidget(serverLabel);
            serverLayout->addWidget(serverEdit);
            dialogLayout->addLayout(serverLayout);

            // 端口输入
            QHBoxLayout *portLayout = new QHBoxLayout();
            QLabel *portLabel = new QLabel("端口号:", inputDialog);
            QLineEdit *portEdit = new QLineEdit(inputDialog);
            portEdit->setPlaceholderText("例如: 12347");
            portEdit->setText("12347"); // 默认值
            portLayout->addWidget(portLabel);
            portLayout->addWidget(portEdit);
            dialogLayout->addLayout(portLayout);

            // 会议ID输入（可选）
            QHBoxLayout *meetingIdLayout = new QHBoxLayout();
            QLabel *meetingIdLabel = new QLabel("会议ID:", inputDialog);
            QLineEdit *meetingIdEdit = new QLineEdit(inputDialog);
            meetingIdEdit->setPlaceholderText("可选: 输入会议ID");
            meetingIdLayout->addWidget(meetingIdLabel);
            meetingIdLayout->addWidget(meetingIdEdit);
            dialogLayout->addLayout(meetingIdLayout);

            // 按钮区域
            QHBoxLayout *buttonDialogLayout = new QHBoxLayout();
            QPushButton *cancelBtn = new QPushButton("取消", inputDialog);
            QPushButton *confirmBtn = new QPushButton("确认加入", inputDialog);

            cancelBtn->setStyleSheet("padding: 8px 16px;");
            confirmBtn->setStyleSheet(
                "QPushButton { background-color: #10b981; color: white; padding: 8px 16px; border-radius: 4px; }"
                "QPushButton:hover { background-color: #059669; }"
                );

            buttonDialogLayout->addStretch();
            buttonDialogLayout->addWidget(cancelBtn);
            buttonDialogLayout->addWidget(confirmBtn);
            dialogLayout->addLayout(buttonDialogLayout);

            // 连接按钮信号
            connect(cancelBtn, &QPushButton::clicked, inputDialog, &QDialog::reject);
            connect(confirmBtn, &QPushButton::clicked, inputDialog, [=]() {
                QString server = serverEdit->text().trimmed();
                QString portStr = portEdit->text().trimmed();
                QString meetingId = meetingIdEdit->text().trimmed();

                if (server.isEmpty()) {
                    QMessageBox::warning(inputDialog, "输入错误", "服务器地址不能为空");
                    return;
                }

                if (portStr.isEmpty()) {
                    QMessageBox::warning(inputDialog, "输入错误", "端口号不能为空");
                    return;
                }

                bool ok;
                int port = portStr.toInt(&ok);
                if (!ok || port < 1 || port > 65535) {
                    QMessageBox::warning(inputDialog, "输入错误", "请输入有效的端口号 (1-65535)");
                    return;
                }

                inputDialog->accept();

                // 创建并显示独立的会议对话框，传入服务器信息
                if (!meetingDialog) {
                    meetingDialog = new MeetingDialog("factory_user", server, port, meetingId, this);
                    connect(meetingDialog, &MeetingDialog::meetingClosed, this, [=]() {
                        meetingDialog->deleteLater();
                        meetingDialog = nullptr;
                    });
                }
                meetingDialog->show();
                meetingDialog->raise();
                meetingDialog->activateWindow();
            });

            // 显示对话框
            if (inputDialog->exec() == QDialog::Accepted) {
                statusLabel->setText("会议状态: 已打开独立会议窗口");
            }
        });

        buttonLayout->addWidget(createMeetingBtn);
        buttonLayout->addWidget(openMeetingWindowBtn);
        buttonLayout->addStretch();

        panelLayout->addLayout(buttonLayout);

        // 会议信息区域
        QGroupBox *infoGroup = new QGroupBox("会议信息", this);
        infoGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 15px; }");
        QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);

        QLabel *infoLabel = new QLabel(
            "功能说明:\n"
            "• 创建会议: 发起一个新的远程会议会话\n"
            "• 加入会议: 通过会议ID加入现有会议\n"
            "• 打开独立窗口: 在独立窗口中运行完整会议功能\n\n"
            "会议功能包括:\n"
            "• 实时音视频通话\n"
            "• 文字聊天\n"
            "• 屏幕共享\n"
            "• 文件传输", this);
        infoLabel->setStyleSheet("font-size: 12px; color: #64748b;");
        infoLabel->setWordWrap(true);

        infoLayout->addWidget(infoLabel);
        panelLayout->addWidget(infoGroup);

        contentLayout->addWidget(meetingPanel);

        // 连接按钮信号：创建会议
        connect(createMeetingBtn, &QPushButton::clicked, this, [=]() {
            statusLabel->setText("会议状态: 正在创建会议...");
            QMessageBox::information(this, "创建会议", "会议创建功能正在开发中");
        });
    }else if (featureName == "newWorkOrder") {

        QLabel *title = new QLabel("新建工单", this);
        title->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; margin-bottom: 20px;");
        contentLayout->addWidget(title);

        auto form = new QWidget(this);
        form->setStyleSheet("background-color:#f8fafc; border-radius:8px; padding:20px;");
        auto fy = new QVBoxLayout(form);

        auto row1 = new QHBoxLayout();
        row1->addWidget(new QLabel("标题:", form));
        newTitleEdit_ = new QLineEdit(form);
        newTitleEdit_->setPlaceholderText("不要包含 | 字符");
        row1->addWidget(newTitleEdit_, 1);
        fy->addLayout(row1);

        auto row2 = new QHBoxLayout();
        row2->addWidget(new QLabel("优先级:", form));
        newPrioBox_ = new QComboBox(form);
        newPrioBox_->addItems({"低","中","高"});
        row2->addWidget(newPrioBox_);
        fy->addLayout(row2);

        auto row3 = new QVBoxLayout();
        row3->addWidget(new QLabel("问题描述:", form));
        newDescEdit_ = new QTextEdit(form);
        newDescEdit_->setPlaceholderText("不要包含 | 字符");
        row3->addWidget(newDescEdit_);
        fy->addLayout(row3);

        auto rowBtn = new QHBoxLayout();
        auto btn = new QPushButton("创建工单", form);
        newResultLabel_ = new QLabel("", form);
        newResultLabel_->setStyleSheet("color:#0ea5e9;");
        rowBtn->addWidget(btn);
        rowBtn->addWidget(newResultLabel_, 1);
        fy->addLayout(rowBtn);

        connect(btn, &QPushButton::clicked, this, &MainInterfaceDialog::onCreateTicketClicked);

        contentLayout->addWidget(form);
    }else if (featureName == "newWorkOrder") {
        // 使用独立的WorkOrderDialog类
        WorkOrderDialog *workOrderDialog = new WorkOrderDialog(this);
        m_workOrderDialog = workOrderDialog; // 保存引用
        
        // 连接创建工单信号
        connect(workOrderDialog, &WorkOrderDialog::createTicket, this, &MainInterfaceDialog::onCreateTicketClicked);
        
        contentLayout->addWidget(workOrderDialog);
    } else if (featureName == "textMessage") {
        // 联系专家 - 聊天界面
        ChatDialog *chatDialog = new ChatDialog(this);
        m_chatDialog = chatDialog; // 保存引用
        
        // 设置连接信息
        if (m_socket) {
            chatDialog->setConnection(m_socket, m_username);
        }
        
        // 连接聊天相关信号
        connect(chatDialog, &ChatDialog::joinChatRequested, this, [this](const QString& workOrderId) {
            // 发送加入聊天请求到服务器
            if (m_socket && m_socket->isOpen()) {
                QString request = QString("ORDER|JOIN|%1\n").arg(workOrderId);
                m_socket->write(request.toUtf8());
                m_socket->flush();
            }
        });
        
        connect(chatDialog, &ChatDialog::messageSent, this, [this](const QString& workOrderId, const QString& message) {
            // 发送消息到服务器，使用与专家端一致的格式
            if (m_socket && m_socket->isOpen()) {
                // 移除消息中的换行符，替换竖线符号
                QString safeMessage = message.trimmed();
                safeMessage.replace('\n', ' ').replace('\r', ' ').replace('|', QChar(0xFF5C));
                
                // 使用专家端的格式：CHAT|SEND|工单ID|用户名|消息内容
                QString request = QString("CHAT|SEND|%1|%2|%3\n").arg(workOrderId, m_username, safeMessage);
                m_socket->write(request.toUtf8());
                m_socket->flush();
                
                // 不再在本地立即添加消息，等待服务器返回确认后再显示
            }
        });
        
        contentLayout->addWidget(chatDialog, 1);
    } else {
        // 其他功能页面
        QLabel *titleLabel = new QLabel(displayName, this);
        titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b; margin-bottom: 20px;");
        contentLayout->addWidget(titleLabel);

        QLabel *contentLabel = new QLabel(QString("%1 功能正在开发中...").arg(displayName), this);
        contentLabel->setStyleSheet("font-size: 16px; color: #64748b;");
        contentLayout->addWidget(contentLabel);

        contentLayout->addStretch();
    }

    return contentWidget;
}

// 侧边栏按钮点击事件处理
void MainInterfaceDialog::on_loginButton_clicked()
{
    createAndShowFeaturePage("login", "账号信息");
}

void MainInterfaceDialog::on_workOrderButton_clicked()
{
    createAndShowFeaturePage("workOrder", "工单管理");
}

void MainInterfaceDialog::on_newWorkOrderButton_clicked()
{
    createAndShowFeaturePage("newWorkOrder", "新建工单");
}

void MainInterfaceDialog::on_workOrderListButton_clicked()
{
    createAndShowFeaturePage("workOrderList", "工单列表");
}

void MainInterfaceDialog::on_communicationButton_clicked()
{
    createAndShowFeaturePage("communication", "通信功能");
}

void MainInterfaceDialog::on_textMessageButton_clicked()
{
    createAndShowFeaturePage("textMessage", "联系专家");
}

void MainInterfaceDialog::on_joinMeetingButton_clicked()
{
    createAndShowFeaturePage("joinMeeting", "在线会议");
}

void MainInterfaceDialog::on_deviceMonitorButton_clicked()
{
    createAndShowFeaturePage("deviceMonitor", "设备监控");
}

void MainInterfaceDialog::on_realTimeDataButton_clicked()
{
    createAndShowFeaturePage("realTimeData", "实时数据");
    // 点击实时数据页面时启动定时器
    startDeviceMonitorTimer();
}

void MainInterfaceDialog::on_deviceRecordingButton_clicked()
{
    createAndShowFeaturePage("deviceRecording", "设备录制");
}



void MainInterfaceDialog::on_knowledgeBaseButton_clicked()
{
    createAndShowFeaturePage("knowledgeBase", "企业知识库");
}

void MainInterfaceDialog::on_tabBar_currentChanged(int index) {
    if (index >= 0 && index < m_tabFeatureNames.size()) {
        QString featureName = m_tabFeatureNames.at(index);
        int pageIndex = m_featurePageMap.value(featureName, -1);
        if (pageIndex != -1 && pageIndex < ui->stackedWidget->count()) {
            ui->stackedWidget->setCurrentIndex(pageIndex);
        }
    }
}

void MainInterfaceDialog::on_tabBar_tabCloseRequested(int index)
{
    if (index < 0 || index >= m_tabFeatureNames.size()) return;

    QString featureName = m_tabFeatureNames.at(index);
    
    // 检查是否是实时数据页面，如果是则停止定时器
    if (featureName == "realTimeData") {
        stopDeviceMonitorTimer();
    }
    
    m_tabFeatureNames.removeAt(index);
    int pageIndex = m_featurePageMap.take(featureName);

    ui->tabBar->removeTab(index);
    QWidget* pageWidget = ui->stackedWidget->widget(pageIndex);
    ui->stackedWidget->removeWidget(pageWidget);
    delete pageWidget;

    for(auto it = m_featurePageMap.begin(); it != m_featurePageMap.end(); ++it) {
        if(it.value() > pageIndex) {
            it.value()--;
        }
    }

    if (ui->tabBar->count() == 0) {
        createEmptyStatePage();
    }
}

void MainInterfaceDialog::setConnection(QTcpSocket* socket, const QString& username) {
    m_socket = socket;
    m_username = username;
    if (!m_socket) return;

    // 若不在 GUI 线程，移回 GUI 线程（防止所在线程没有事件循环导致 readyRead 不触发）
    if (m_socket->thread() != qApp->thread()) {
        m_socket->moveToThread(qApp->thread());
    }

    // 先断开旧连接，确保没有其它对象在读这个 socket（特别是登录窗口）
    QObject::disconnect(m_socket, nullptr, nullptr, nullptr);

    // 直连 readyRead，并强制调用本类的槽
    connect(m_socket, &QTcpSocket::readyRead, this, [this]{
        this->onSocketReadyRead();
    }, static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));

    // 可选：监听状态/错误
    connect(m_socket, &QTcpSocket::stateChanged, this, [](QAbstractSocket::SocketState){
        // 移除了状态调试输出
    });
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, [](auto){ /* 移除了错误调试输出 */ });

    // 1秒后发一条测试命令
    QTimer::singleShot(1000, this, [this]{
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write("PING|TEST\n");
            m_socket->flush();
        }
    });

    // 1.5秒后兜底轮询，验证是否真的有数据却没有触发 readyRead
    QTimer::singleShot(1500, this, [this]{
        if (!m_socket) return;
        if (m_socket->bytesAvailable()==0) m_socket->waitForReadyRead(500);
        QByteArray all = m_socket->readAll();
        if (!all.isEmpty()) {
            m_recvBuf += all;
            // 复用 onSocketReadyRead 的逐行解析逻辑
            this->onSocketReadyRead();
        }
    });

    // 启动一个轻量泵，每100ms检查一次是否有数据
    if (!m_netPump) {
        m_netPump = new QTimer(this);
        m_netPump->setInterval(100);
        connect(m_netPump, &QTimer::timeout, this, [this]{
            if (!m_socket) return;
            // 如果系统没发readyRead，就主动等一小会儿
            if (m_socket->bytesAvailable() == 0)
                m_socket->waitForReadyRead(1); // 1ms，不会卡界面
            if (m_socket->bytesAvailable() > 0) {
                onSocketReadyRead(); // 统一在这里处理
            }
        });
    }
    m_netPump->start();
}

void MainInterfaceDialog::onSocketReadyRead() {
    if (!m_socket) return;

    m_recvBuf += m_socket->readAll();

    int pos;
    while ((pos = m_recvBuf.indexOf('\n')) != -1) {
        QByteArray one = m_recvBuf.left(pos);
        m_recvBuf.remove(0, pos + 1);

        const QString line = QString::fromUtf8(one).trimmed();
        if (line.isEmpty()) continue;

        const QStringList p = line.split('|');
        if (p.size() < 2) continue;
        const QString type = p[0];

        if (type == "CREATE_TICKET") {
            // 处理服务端返回的CREATE_TICKET响应
            if (p.size() >= 2 && p[1] == "SUCCESS") {
                const QString id = p.size() >= 3 ? p[2] : "";
                // 总是显示消息框通知用户工单创建成功
                QMessageBox::information(this, "创建成功", QString("工单编号: %1").arg(id));
            }
        } else if (type == "ORDER" && p.size() >= 4 && p[1] == "ACCEPTED") {
            // 你的原逻辑...
        } else if (type == "TICKET_CREATED") {
            // 保留原有的TICKET_CREATED处理逻辑，以兼容可能的其他来源
            const QString id = (p.size()>=3 && p[1]=="SUCCESS") ? p[2] : (p.size()>=2 ? p[1] : "");
            QMessageBox::information(this, "创建成功", QString("工单编号: %1").arg(id));
        } else if (type == "DEVICE_DATA" && p.size() >= 8) {
            const QString deviceId = "device_001";
            updateDeviceMonitorData(deviceId,
                                    p[1].toDouble(), p[2].toDouble(), p[3].toDouble(),
                                    p[4].toDouble(), p[5].toDouble(), p[6].toDouble(), p[7].toInt());
        } else if (type == "TELE" && p.size() >= 4 && p[1] == "DATA") {
            const QString jsonData = p.mid(3).join("|"); // 兼容 JSON 中出现 '|' 的情况

            QJsonParseError err;
            const auto doc = QJsonDocument::fromJson(jsonData.toUtf8(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                continue;
            }
            const QJsonObject obj = doc.object();
            auto num = [&](const char* k)->double {
                if (!obj.contains(k)) return 0.0;
                const QJsonValue v = obj.value(k);
                return v.isString() ? v.toString().toDouble() : v.toDouble(0.0);
            };

            const QString deviceId = "device_001";
            updateDeviceMonitorData(deviceId,
                                    num("temperature"),
                                    num("pressure"),
                                    num("vibration"),
                                    num("current"),
                                    num("voltage"),
                                    num("speed"),
                                    obj.contains("status") ? obj["status"].toInt(1) : 1);
        } else if (type == "ORDER" && p.size() >= 3 && p[1] == "JOIN") {
            // 处理加入聊天的响应
            if (p.size() >= 3 && p[2] == "OK" && p.size() >= 4) {
                // 加入聊天成功
                const QString workOrderId = p[3];
                if (m_chatDialog) {
                    m_chatDialog->showChatSuccess(workOrderId);
                }
            } else if (p.size() >= 3 && p[2] == "ERR" && p.size() >= 4) {
                // 加入聊天失败
                const QString errorMessage = p[3];
                if (m_chatDialog) {
                    m_chatDialog->showChatError(errorMessage);
                }
            }
        } else if (type == "MESSAGE" && p.size() >= 4 && p[1] == "RECEIVE") {
            // 处理收到的消息（旧格式，保持兼容）
            if (p.size() >= 5) {
                const QString workOrderId = p[2];
                const QString sender = p[3];
                const QString message = p.mid(4).join("|"); // 兼容消息中出现 '|' 的情况
                if (m_chatDialog) {
                    m_chatDialog->appendMessage(sender, message, false);
                }
            }
        } else if (type == "CHAT" && p.size() >= 5 && p[1] == "MSG") {
            // 处理专家端格式的消息
            if (p.size() >= 6) {
                const QString workOrderId = p[2];
                const QString sender = p[3];
                // 跳过时间戳 p[4]
                const QString message = p.mid(5).join("|"); // 兼容消息中出现 '|' 的情况
                if (m_chatDialog) {
                    m_chatDialog->appendMessage(sender, message, false);
                }
            }
        }
    }
}

void MainInterfaceDialog::onCreateTicketClicked() {
    if (!m_socket) {
        QMessageBox::warning(this, "未连接", "尚未连接到服务器");
        return;
    }
    
    // 从WorkOrderDialog获取工单信息
    if (!m_workOrderDialog) {
        QMessageBox::warning(this, "错误", "工单对话框未初始化");
        return;
    }
    
    const QString title = m_workOrderDialog->getTitle().trimmed();
    const QString desc = m_workOrderDialog->getDescription().trimmed();
    const QString prio = m_workOrderDialog->getPriority();
    
    if (title.isEmpty() || desc.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写标题和描述");
        return;
    }
    
    auto sanitize = [](QString s){ s.replace("\n"," ").replace("\r"," ").replace("|"," "); return s; };
    const QString line = QString("CREATE_TICKET|%1|%2|%3")
                             .arg(sanitize(title), sanitize(desc), sanitize(prio));
    m_socket->write(line.toUtf8() + '\n');
    m_socket->flush();
}

void MainInterfaceDialog::updateDeviceMonitorData(const QString &deviceId, double temperature, double pressure, double vibration, double current, double voltage, double speed, int status){
    if (!m_deviceMonitorPanel) {
        // 缓存数据
        DeviceParams params;
        params.temperature = temperature;
        params.pressure = pressure;
        params.vibration = vibration;
        params.current = current;
        params.voltage = voltage;
        params.speed = speed;
        params.status = status;
        params.lastUpdate = QDateTime::currentDateTime();
        m_deviceDataCache[deviceId] = params;
        return; // 如果监控面板未初始化，则缓存数据后返回
    }
    
    // 获取设备管理器
    DeviceManager& deviceManager = m_deviceMonitorPanel->getDeviceManager();
    
    // 获取或创建设备
    DeviceInfo* device = deviceManager.getDevice(deviceId);
    if (device) {
            // 状态值反转：服务器返回的1表示正常，0表示异常，但我们的枚举定义相反
            device->setStatus(status == 1 ? DeviceStatus::NORMAL : DeviceStatus::ABNORMAL);
            device->updateParameter("温度", temperature);
            device->updateParameter("压力", pressure);
            device->updateParameter("振动", vibration);
            device->updateParameter("电流", current);
            device->updateParameter("电压", voltage);
            device->updateParameter("转速", speed);
    } else {
        // 创建新设备并添加参数
        // 状态值反转：服务器返回的1表示正常，0表示异常，但我们的枚举定义相反
        DeviceInfo newDevice(deviceId, QString("设备") + deviceId, status == 1 ? DeviceStatus::NORMAL : DeviceStatus::ABNORMAL);
        newDevice.addParameter(DeviceParameter("温度", temperature, "°C", 20.0, 100.0));
        newDevice.addParameter(DeviceParameter("压力", pressure, "kPa", 80.0, 150.0));
        newDevice.addParameter(DeviceParameter("振动", vibration, "mm/s", 0.1, 6.0));
        newDevice.addParameter(DeviceParameter("电流", current, "A", 3.0, 20.0));
        newDevice.addParameter(DeviceParameter("电压", voltage, "V", 200.0, 240.0));
        newDevice.addParameter(DeviceParameter("转速", speed, "RPM", 1300.0, 1700.0));
        deviceManager.addDevice(newDevice);
        // 不调用initialize()以避免重复创建控件
        // 只更新UI数据
        m_deviceMonitorPanel->updateData();
    }
    
    // 强制更新设备监控面板的UI - 直接调用public方法
    if (m_deviceMonitorPanel) {
        // 更新最后收到参数的时间
        m_deviceMonitorPanel->updateLastParameterTime(deviceId);
        m_deviceMonitorPanel->updateData();
    }
}

// 检查并应用缓存的设备数据
void MainInterfaceDialog::applyDeviceDataCache() {
    if (!m_deviceMonitorPanel || m_deviceDataCache.isEmpty()) {
        return;
    }
    
    // 遍历缓存的所有设备数据并应用
    for (auto it = m_deviceDataCache.constBegin(); it != m_deviceDataCache.constEnd(); ++it) {
        const QString &deviceId = it.key();
        const DeviceParams &params = it.value();
        
        // 调用updateDeviceMonitorData函数应用缓存的数据
        // 这会跳过缓存逻辑（因为m_deviceMonitorPanel现在不为空）
        updateDeviceMonitorData(deviceId, params.temperature, params.pressure, 
                               params.vibration, params.current, params.voltage, 
                               params.speed, params.status);
    }
    
    // 清空缓存，因为数据已经被应用
    m_deviceDataCache.clear();
}

void MainInterfaceDialog::startDeviceMonitorTimer()
{
    // 启动设备监控定时器
    if (m_deviceMonitorPanel) {
        m_deviceMonitorPanel->startTimer();
    }
}

void MainInterfaceDialog::stopDeviceMonitorTimer()
{
    // 停止设备监控定时器
    if (m_deviceMonitorPanel) {
        m_deviceMonitorPanel->stopTimer();
    }
}

// 辅助函数：从JSON字符串中提取指定键的值
QString MainInterfaceDialog::getValueFromJson(const QString &json, const QString &key, const QString &defaultValue)
{
    // 简单的字符串解析方法，用于从JSON中提取指定键的值
    // 在实际应用中，可以使用QJsonDocument等工具进行更复杂的JSON解析
    
    // 查找键的位置
    int keyPos = json.indexOf('"' + key + '"');
    if (keyPos == -1) {
        return defaultValue;
    }
    
    // 查找冒号位置
    int colonPos = json.indexOf(':', keyPos);
    if (colonPos == -1) {
        return defaultValue;
    }
    
    // 跳过冒号和空格
    colonPos++;
    while (colonPos < json.length() && json.at(colonPos) == ' ') {
        colonPos++;
    }
    
    // 查找值的开始和结束位置
    int startPos = colonPos;
    int endPos = startPos;
    
    // 如果值是字符串，查找下一个引号
    if (json.at(startPos) == '"') {
        startPos++;
        endPos = json.indexOf('"', startPos);
        if (endPos == -1) {
            return defaultValue;
        }
    } else {
        // 如果值是数字或布尔值，查找下一个逗号或右大括号
        while (endPos < json.length() && 
               json.at(endPos) != ',' && 
               json.at(endPos) != '}' && 
               json.at(endPos) != ' ') {
            endPos++;
        }
    }
    
    // 提取值并去除空格
    QString value = json.mid(startPos, endPos - startPos).trimmed();
    
    return value.isEmpty() ? defaultValue : value;
}
