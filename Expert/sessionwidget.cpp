#include "sessionwidget.h"
#include "ui_sessionwidget.h"

#include "ticketspagewidget.h"
#include "chatmessagewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QDateTime>
#include <QJsonValue>
#include <climits>
#include <algorithm>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QScrollBar>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QAbstractButton>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>

#include "tcpclient.h"
#include "authmanager.h"
#include "workordermanager.h"
#include "telemetryclient.h"
#include "chatclient.h"
#include "ticketsclient.h"

using namespace QtCharts;

namespace {
QString fmtDuration(int secs) {
    int m = secs / 60, s = secs % 60, h = m / 60; m %= 60;
    return h ? QString("%1:%2:%3").arg(h,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0'))
             : QString("%1:%2").arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0'));
}
}

SessionWidget::SessionWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SessionWidget)
{
    ui->setupUi(this);

    // 发送按钮图标
    ui->chatSendBtn->setIcon(createSendIcon());
    ui->chatSendBtn->setIconSize(QSize(24, 24));

    // 聊天内容区顶部对齐
    ui->chatContentLayout->addStretch(1);

    // 卡片阴影
    auto addShadow = [this](QWidget* w) {
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(25);
        shadow->setOffset(0, 5);
        shadow->setColor(QColor(0, 0, 0, 50));
        w->setGraphicsEffect(shadow);
    };
    addShadow(ui->createMeetingCard);
    addShadow(ui->joinMeetingCard);

    // 卡片图标
    ui->createMeetingIconLabel->setPixmap(createCameraIcon().pixmap(QSize(64, 64)));
    ui->joinMeetingIconLabel->setPixmap(createJoinIcon().pixmap(QSize(64, 64)));

    // 侧栏顶部图标
    ui->sidebarIconLabel->setPixmap(createWrenchIcon().pixmap(QSize(48, 48)));

    applyStyles();
    setupCharts();
    setupConnections();

    // 在左侧栏插入“录制”按钮（位于“账号管理”按钮上方）
    {
        auto* sidebarLayout = qobject_cast<QVBoxLayout*>(ui->sidebarFrame->layout());
        recordBtn_ = new QPushButton(tr("开始录制"), ui->sidebarFrame);
        recordBtn_->setObjectName("btnRecord");
        recordBtn_->setCursor(Qt::PointingHandCursor); // 添加手型光标

        if (sidebarLayout) {
            int idx = sidebarLayout->indexOf(ui->navButton_Account);
            if (idx >= 0) sidebarLayout->insertWidget(idx, recordBtn_);
            else          sidebarLayout->addWidget(recordBtn_);
        }
        
        // 连接录制按钮点击信号
        const bool ok = connect(recordBtn_, &QPushButton::clicked, this, [this]{
            qDebug() << "[UI] recordBtn_ clicked";
            onRecordButtonClicked();
        });
        Q_ASSERT(ok);

        // 连接录屏器信号
        connect(&recorder_, SIGNAL(started(QString)),      this, SLOT(onRecordStarted(QString)));
        connect(&recorder_, SIGNAL(stopped(QString)),      this, SLOT(onRecordStopped(QString)));
        connect(&recorder_, SIGNAL(error(QString)),        this, SLOT(onRecordError(QString)));
        connect(&recorder_, SIGNAL(progressSec(int)),      this, SLOT(onRecordProgress(int)));
    }
}

SessionWidget::~SessionWidget()
{
    if (recorder_.isRecording()) recorder_.stop();
    delete ui;
}

void SessionWidget::applyStyles()
{
    this->setStyleSheet(R"(
        #sidebarFrame {
            background-color: #2c3e50;
        }

        #sidebarTitleLabel {
            color: #ecf0f1;
            font-weight: bold;
            font-size: 14px;
        }
        #connectionStatusIndicator {
            min-width: 12px;
            max-width: 12px;
            min-height: 12px;
            max-height: 12px;
            border-radius: 6px;
            background-color: #e74c3c;
        }
        #connectionStatusText {
            color: #95a5a6;
        }
        #connectionStatusIndicator[status="connected"] { background-color: #2ecc71; }
        #connectionStatusIndicator[status="disconnected"] { background-color: #e74c3c; }
        #connectionStatusIndicator[status="connecting"] { background-color: #f39c12; }

        [objectName^="categoryLabel"] {
            color: #95a5a6;
            font-size: 12px;
            font-weight: bold;
            margin-top: 15px;
            margin-bottom: 5px;
            margin-left: 5px;
            background-color: transparent;
        }
        #sidebarFrame QPushButton {
            color: white;
            background-color: #34495e;
            border: none;
            padding: 10px;
            text-align: left;
            border-radius: 4px;
            font-weight: bold;
            font-size: 13px;
        }
        #sidebarFrame QPushButton:hover { background-color: #4a627a; }
        #sidebarFrame QPushButton:checked { background-color: #3498db; color: white; }

        /* 录制中高亮（可选） */
        #sidebarFrame QPushButton[recording="true"] { background-color: #e74c3c; color: #fff; }

        QGroupBox { font-weight: bold; font-size: 14px; }
        QWidget { background-color: #ecf0f1; }
        QSplitter::handle { background-color: #bdc3c7; }

        #joinOrderTitle { font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 20px; }
        #joinBtn { background-color: #2ecc71; color: white; padding: 8px 24px; border-radius: 4px; border: none; }
        #joinBtn:hover { background-color: #27ae60; }
        #leaveBtn { background-color: #e74c3c; color: white; padding: 8px 24px; border-radius: 4px; border: none; }
        #leaveBtn:hover { background-color: #c0392b; }

        #chartContainer QPushButton {
            background-color: #bdc3c7; color: #2c3e50; border: 1px solid #95a5a6;
            padding: 5px 15px; border-radius: 4px;
        }
        #chartContainer QPushButton:hover { background-color: #95a5a6; }
        #chartContainer QPushButton:checked { background-color: #3498db; color: white; border-color: #2980b9; }

        #chatHeaderLabel { background-color: #ffffff; color: #2c3e50; font-weight: bold; padding: 10px; border-bottom: 1px solid #e0e0e0; }
        #chatScrollArea { border: none; }
        #chatContentWidget { background-color: #ffffff; }
        #chatInputFrame { background-color: #f5f7fa; border-top: 1px solid #e0e0e0; padding: 8px; }
        #chatInputEdit { background-color: #ffffff; border: 1px solid #dcdfe6; border-radius: 8px; padding: 8px; max-height: 80px; }
        #chatSendBtn { background-color: #3498db; border: none; border-radius: 8px; min-width: 36px; max-width: 36px; min-height: 36px; max-height: 36px; }
        #chatSendBtn:disabled { background-color: #aab7c4; }

        #bubbleContainer { border-radius: 10px; }
        ChatMessageWidget[alignment="self"] #bubbleContainer { background-color: #3498db; }
        ChatMessageWidget[alignment="other"] #bubbleContainer { background-color: #e5e5ea; }
        ChatMessageWidget[alignment="self"] #messageLabel,
        ChatMessageWidget[alignment="self"] #senderLabel,
        ChatMessageWidget[alignment="self"] #timestampLabel { color: white; }
        ChatMessageWidget[alignment="other"] #messageLabel,
        ChatMessageWidget[alignment="other"] #senderLabel { color: #2c3e50; }
        ChatMessageWidget[alignment="system"] #messageLabel {
            color: #8e8e93; font-size: 12px; background-color: #f0f0f0; border-radius: 8px; padding: 4px 10px; margin: 4px 40px;
        }

        ChatMessageWidget #messageLabel, ChatMessageWidget #senderLabel, ChatMessageWidget #timestampLabel { background-color: transparent; }

        #avatarLabel { background-color: #bdc3c7; color: white; font-weight: bold; min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px; border-radius: 20px; qproperty-alignment: 'AlignCenter'; }
        #senderLabel { font-size: 12px; font-weight: bold; }
        #timestampLabel { color: #c7c7cd; font-size: 10px; }

        #page_Meeting { background-color: #f5f7fa; }
        #createMeetingCard, #joinMeetingCard { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 8px; }
        #createMeetingIconLabel, #joinMeetingIconLabel { margin-top: 10px; margin-bottom: 10px; }
        #createMeetingTitleLabel, #joinMeetingTitleLabel { font-size: 18px; font-weight: bold; color: #2c3e50; }
        #createMeetingDescLabel { color: #8e8e93; }
        #joinMeetingIdEdit { min-height: 30px; padding: 0 8px; border-radius: 4px; border: 1px solid #dcdfe6; margin: 10px 20px; }
        #createMeetingCardBtn, #joinMeetingCardBtn { background-color: #3498db; color: white; border: none; border-radius: 4px; padding: 10px 20px; font-weight: bold; }
        #joinMeetingCardBtn:disabled { background-color: #aab7c4; }
    )");
}

void SessionWidget::setupCharts() {
    tempSeries_  = new QLineSeries(this); tempSeries_->setName(tr("温度(°C)"));
    pressSeries_ = new QLineSeries(this); pressSeries_->setName(tr("压力(kPa)"));
    vibSeries_   = new QLineSeries(this); vibSeries_->setName(tr("振动(mm/s)"));
    currSeries_  = new QLineSeries(this); currSeries_->setName(tr("电流(A)"));
    voltSeries_  = new QLineSeries(this); voltSeries_->setName(tr("电压(V)"));
    speedSeries_ = new QLineSeries(this); speedSeries_->setName(tr("转速(RPM)"));

    tempSeries_->setColor(QColor("#e74c3c"));
    pressSeries_->setColor(QColor("#3498db"));
    vibSeries_->setColor(QColor("#e67e22"));
    currSeries_->setColor(QColor("#27ae60"));
    voltSeries_->setColor(QColor("#9b59b6"));
    speedSeries_->setColor(QColor("#7f8c8d"));

    chart_ = new QChart();
    chart_->addSeries(tempSeries_);
    chart_->addSeries(pressSeries_);
    chart_->addSeries(vibSeries_);
    chart_->addSeries(currSeries_);
    chart_->addSeries(voltSeries_);
    chart_->addSeries(speedSeries_);
    chart_->setTitle(tr("实时指标曲线"));
    chart_->legend()->setAlignment(Qt::AlignBottom);

    axisX_ = new QDateTimeAxis(this);
    axisX_->setFormat("HH:mm:ss");
    axisX_->setTitleText(tr("时间"));

    axisY_ = new QValueAxis(this);
    axisY_->setTitleText(tr("数值"));
    axisY_->setLabelFormat("%.1f");

    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);

    for (auto s : {tempSeries_, pressSeries_, vibSeries_, currSeries_, voltSeries_, speedSeries_}) {
        s->attachAxis(axisX_);
        s->attachAxis(axisY_);
    }

    chartView_ = new QChartView(chart_);
    chartView_->setRenderHint(QPainter::Antialiasing);

    ui->chartPlaceholder->setLayout(new QVBoxLayout());
    ui->chartPlaceholder->layout()->setContentsMargins(0,0,0,0);
    ui->chartPlaceholder->layout()->addWidget(chartView_);

    auto makeBtn = [&](const QString& text){
        auto* btn = new QPushButton(text, this);
        btn->setCheckable(true);
        ui->metricsButtonsLayout->addWidget(btn);
        return btn;
    };
    auto* btnTemp_  = makeBtn(tr("温度"));
    auto* btnPress_ = makeBtn(tr("压力"));
    auto* btnVib_   = makeBtn(tr("振动"));
    auto* btnCurr_  = makeBtn(tr("电流"));
    auto* btnVolt_  = makeBtn(tr("电压"));
    auto* btnSpeed_ = makeBtn(tr("转速"));
    ui->metricsButtonsLayout->addStretch();

    btnTemp_->setChecked(true);
    btnPress_->setChecked(true);
    tempSeries_->setVisible(true);
    pressSeries_->setVisible(true);
    vibSeries_->setVisible(false);
    currSeries_->setVisible(false);
    voltSeries_->setVisible(false);
    speedSeries_->setVisible(false);

    auto bindToggle = [&](QPushButton* btn, QLineSeries* s){
        connect(btn, &QPushButton::toggled, this, [=](bool on){
            s->setVisible(on);
            refreshAxisRange();
        });
    };
    bindToggle(btnTemp_,  tempSeries_);
    bindToggle(btnPress_, pressSeries_);
    bindToggle(btnVib_,   vibSeries_);
    bindToggle(btnCurr_,  currSeries_);
    bindToggle(btnVolt_,  voltSeries_);
    bindToggle(btnSpeed_, speedSeries_);
}

void SessionWidget::setupConnections()
{
    auto* navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    navGroup->addButton(ui->navButton_DeviceData);
    navGroup->addButton(ui->navButton_JoinOrder);
    navGroup->addButton(ui->navButton_History);
    navGroup->addButton(ui->navButton_Chat);
    navGroup->addButton(ui->navButton_Meeting);
    navGroup->addButton(ui->navButton_Account);
    connect(navGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onNavigationChanged()));

    connect(ui->joinBtn,  &QPushButton::clicked, this, &SessionWidget::onJoinClicked);
    connect(ui->leaveBtn, &QPushButton::clicked, this, &SessionWidget::onLeaveClicked);
    connect(ui->chatSendBtn, &QPushButton::clicked, this, &SessionWidget::onSendChat);

    connect(ui->createMeetingCardBtn, &QPushButton::clicked, this, &SessionWidget::createMeetingRequested);
    connect(ui->joinMeetingCardBtn, &QPushButton::clicked, this, &SessionWidget::joinMeetingRequested);

    connect(ui->chatInputEdit, &QTextEdit::textChanged, this, [this](){
        ui->chatSendBtn->setEnabled(!ui->chatInputEdit->toPlainText().trimmed().isEmpty());
    });

    connect(ui->joinMeetingIdEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        ui->joinMeetingCardBtn->setEnabled(!text.trimmed().isEmpty());
    });
    ui->joinMeetingCardBtn->setEnabled(false);
}

void SessionWidget::bindServices(TcpClient* tcp,
                                 AuthManager* auth,
                                 WorkOrderManager* orders,
                                 TelemetryClient* telem,
                                 TicketsClient* tickets) {
    tcp_ = tcp; auth_ = auth; orders_ = orders; telem_ = telem;

    if (orders_) {
        connect(orders_, &WorkOrderManager::joined,     this, &SessionWidget::onJoined);
        connect(orders_, &WorkOrderManager::joinFailed, this, &SessionWidget::onJoinFailed);
        connect(orders_, &WorkOrderManager::left,       this, &SessionWidget::onLeft);
    }
    if (telem_) {
        connect(telem_, &TelemetryClient::metricsUpdated, this, &SessionWidget::onMetrics);
        connect(telem_, &TelemetryClient::logAppended,    this, &SessionWidget::onLogLine);
        connect(telem_, &TelemetryClient::faultRaised,    this, &SessionWidget::onFault);
    }

    if(tickets && orders_) {
        ticketsPage_ = new TicketsPageWidget(this);
        ticketsPage_->bindServices(tickets, orders_);
        ui->historyPageLayout->addWidget(ticketsPage_);
    }

    // 初始化一次连接状态
    if (tcp_) {
        updateConnectionStatus(tcp_->isConnected(), tcp_->isConnected() ? tr("在线") : tr("离线"));
    }
}

void SessionWidget::attachChat(ChatClient* chat) {
    chat_ = chat;
    if (chat_) {
        connect(chat_, &ChatClient::messageReceived, this, &SessionWidget::onChatMessage);
    }
}

void SessionWidget::setOrderId(const QString& id, const QString& title) {
    currentOrderId_ = id;
    ui->orderStatusLabel->setText(id.isEmpty() ? tr("未加入") : tr("已加入: %1").arg(id));
    if (id.isEmpty()) {
        ui->chatHeaderLabel->setText(tr("当前工单: 未加入"));
    } else if (title.isEmpty()) {
        ui->chatHeaderLabel->setText(tr("当前工单: %1").arg(id));
    } else {
        ui->chatHeaderLabel->setText(tr("当前工单: %1 - %2").arg(id, title));
    }
    updateUiState();
}

void SessionWidget::onJoinClicked() {
    if (!orders_) return;
    const QString oid = ui->orderEdit->text().trimmed();
    if (oid.isEmpty()) return;
    orders_->join(oid);
}

void SessionWidget::onLeaveClicked() {
    if (!orders_) return;
    orders_->leave();
    if (telem_) telem_->unsubscribe();
    clearChartData();
}

void SessionWidget::onJoined(const QString& orderId, const QString& title) {
    setOrderId(orderId, title);
    if (telem_) telem_->subscribe(orderId);
    clearChartData();
    appendSystemMessage(QString("已加入工单 %1").arg(orderId));
    QMessageBox::information(this, tr("操作成功"), tr("已成功加入工单 %1！").arg(orderId));
}

void SessionWidget::onJoinFailed(const QString& reason) {
    appendSystemMessage(QString("加入失败 - %1").arg(reason));
}

void SessionWidget::onLeft(const QString& orderId) {
    Q_UNUSED(orderId);
    if (recorder_.isRecording()) recorder_.stop(); // 离开即停
    setOrderId(QString());
    if (telem_) telem_->unsubscribe();
    clearChartData();
    appendSystemMessage("已离开工单");
}

void SessionWidget::onMetrics(const QString& orderId, const QJsonObject& m) {
    if (orderId != currentOrderId_) return;
    const qint64 ts = m.value("ts").toVariant().toLongLong();
    auto appendIf = [&](QLineSeries* s, const char* key){ if (m.contains(key)) s->append(ts, m.value(key).toDouble()); };
    appendIf(tempSeries_,  "temperature");
    appendIf(pressSeries_, "pressure");
    appendIf(vibSeries_,   "vibration");
    appendIf(currSeries_,  "current");
    appendIf(voltSeries_,  "voltage");
    if (m.contains("speed")) speedSeries_->append(ts, m.value("speed").toInt());
    auto trim = [&](QLineSeries* s){ if (!s) return; int c = s->count(); if (c > maxPoints_) s->removePoints(0, c - maxPoints_); };
    for (auto s : {tempSeries_, pressSeries_, vibSeries_, currSeries_, voltSeries_, speedSeries_}) trim(s);
    qint64 xmin = LLONG_MAX, xmax = LLONG_MIN;
    for (auto s : {tempSeries_, pressSeries_, vibSeries_, currSeries_, voltSeries_, speedSeries_}) {
        if (!s->isVisible() || s->count() < 1) continue;
        xmin = std::min(xmin, static_cast<qint64>(s->at(0).x()));
        xmax = std::max(xmax, static_cast<qint64>(s->at(s->count()-1).x()));
    }
    if (xmin != LLONG_MAX && xmax != LLONG_MIN) {
        axisX_->setRange(QDateTime::fromMSecsSinceEpoch(xmin), QDateTime::fromMSecsSinceEpoch(xmax));
    }
    refreshAxisRange();
    appendLogLine(makeMetricsLogLine(ts, m));
}

void SessionWidget::onLogLine(const QString& orderId, const QString& line) {
    if (orderId != currentOrderId_) return;
    appendLogLine(line);
}

void SessionWidget::onFault(const QString& orderId, const QString& code, const QString& text, const QString& level) {
    Q_UNUSED(level);
    if (orderId != currentOrderId_) return;
    appendFaultLine(QString("%1: %2").arg(code, text));
}

void SessionWidget::onSendChat() {
    const QString text = ui->chatInputEdit->toPlainText().trimmed();
    if (text.isEmpty() || currentOrderId_.isEmpty()) return;
    if (chat_) chat_->sendText(currentOrderId_, text);
    ui->chatInputEdit->clear();
}

void SessionWidget::onChatMessage(const QString& orderId, const QString& from, const QString& text, qint64 ts) {
    if (orderId != currentOrderId_) return;
    const auto t = QDateTime::fromMSecsSinceEpoch(ts).toString("HH:mm:ss");
    ChatMessageWidget::Alignment alignment = ChatMessageWidget::Other;
    if (auth_ && from == auth_->userId()) alignment = ChatMessageWidget::Self;
    appendChatLine(from, text, t, alignment);
}

void SessionWidget::appendChatLine(const QString &sender, const QString &text, const QString &timestamp, ChatMessageWidget::Alignment alignment)
{
    auto *chatWidget = new ChatMessageWidget(ui->chatContentWidget);
    chatWidget->setMessage(sender, text, timestamp, alignment);

    ui->chatContentLayout->insertWidget(ui->chatContentLayout->count() - 1, chatWidget);

    QTimer::singleShot(0, this, [this](){
        ui->chatScrollArea->verticalScrollBar()->setValue(ui->chatScrollArea->verticalScrollBar()->maximum());
    });
}

void SessionWidget::appendSystemMessage(const QString &text)
{
    auto *chatWidget = new ChatMessageWidget(ui->chatContentWidget);
    chatWidget->setMessage("", text, "", ChatMessageWidget::System);

    ui->chatContentLayout->insertWidget(ui->chatContentLayout->count() - 1, chatWidget);

    QTimer::singleShot(0, this, [this](){
        ui->chatScrollArea->verticalScrollBar()->setValue(ui->chatScrollArea->verticalScrollBar()->maximum());
    });
}

void SessionWidget::appendLogLine(const QString& line)   { ui->logView->appendHtml(formatLogLine(line)); }
void SessionWidget::appendFaultLine(const QString& line) { ui->faultView->appendHtml(formatFaultLine(line)); }

void SessionWidget::refreshAxisRange() {
    double ymin =  1e100, ymax = -1e100;
    auto collect = [&](QLineSeries* s){
        if (!s->isVisible() || s->pointsVector().isEmpty()) return;
        for (const auto& p : s->pointsVector()) {
            ymin = std::min(ymin, p.y());
            ymax = std::max(ymax, p.y());
        }
    };
    for (auto s : {tempSeries_, pressSeries_, vibSeries_, currSeries_, voltSeries_, speedSeries_}) collect(s);
    if (ymin <= ymax && ymin < 1e50) {
        if (ymin == ymax) { ymin -= 1; ymax += 1; }
        axisY_->setRange(ymin - (ymax-ymin)*0.1, ymax + (ymax-ymin)*0.1);
    }
}

void SessionWidget::clearChartData() {
    for (auto s : {tempSeries_, pressSeries_, vibSeries_, currSeries_, voltSeries_, speedSeries_})
        s->clear();
}

QString SessionWidget::makeMetricsLogLine(qint64 ts, const QJsonObject& m) const {
    const QString t = QDateTime::fromMSecsSinceEpoch(ts).toString("HH:mm:ss");
    auto has = [&](const char* k){ return m.contains(k) && !m.value(k).isNull(); };
    QStringList parts;
    parts << QString("[%1]").arg(t);
    if (has("temperature")) parts << QString("T=%1°C").arg(m.value("temperature").toDouble(), 0, 'f', 1);
    if (has("pressure"))    parts << QString("P=%1kPa").arg(m.value("pressure").toDouble(),    0, 'f', 1);
    if (has("vibration"))   parts << QString("V=%1mm/s").arg(m.value("vibration").toDouble(),  0, 'f', 2);
    if (has("current"))     parts << QString("I=%1A").arg(m.value("current").toDouble(),       0, 'f', 1);
    if (has("voltage"))     parts << QString("U=%1V").arg(m.value("voltage").toDouble(),       0, 'f', 1);
    if (has("speed"))       parts << QString("S=%1RPM").arg(m.value("speed").toInt());
    if (has("status")) { const bool st = m.value("status").toInt() != 0 || m.value("status").toBool(); parts << QString("Status=%1").arg(st ? tr("运行") : tr("停止")); }
    return parts.join(' ');
}

QString SessionWidget::formatLogLine(const QString& line) const
{
    QString html = line;
    html.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
    const QString timeColor = "#16a085";
    const QString keyColor = "#2980b9";
    const QString valueColor = "#2c3e50";
    const QString statusOkColor = "#27ae60";
    const QString statusStopColor = "#e67e22";
    html.replace(QRegularExpression("^\\[([^\\]]+)\\]"), QStringLiteral("<font color='%1'>[\\1]</font>").arg(timeColor));
    html.replace(QRegularExpression("([A-Z])=([^\\s]+)"), QStringLiteral("<font color='%1'>\\1</font>=<font color='%2'><b>\\2</b></font>").arg(keyColor, valueColor));
    html.replace(QStringLiteral("Status=<font color='%1'><b>运行</b></font>").arg(valueColor), QStringLiteral("Status=<font color='%1'><b>运行</b></font>").arg(statusOkColor));
    html.replace(QStringLiteral("Status=<font color='%1'><b>停止</b></font>").arg(valueColor), QStringLiteral("Status=<font color='%1'><b>停止</b></font>").arg(statusStopColor));
    return html;
}

QString SessionWidget::formatFaultLine(const QString& line) const
{
    QString html = line;
    html.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
    const QString faultCodeColor = "#c0392b";
    html.replace(QRegularExpression("^([^:]+):"), QStringLiteral("<font color='%1'><b>\\1</b></font>:").arg(faultCodeColor));
    return html;
}

QIcon SessionWidget::createSendIcon() const
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPainterPath path;
    path.moveTo(5, 16);
    path.lineTo(27, 5);
    path.lineTo(19, 27);
    path.lineTo(5, 16);
    painter.drawPath(path);

    painter.drawLine(27, 5, 13, 19);

    return QIcon(pixmap);
}

QIcon SessionWidget::createCameraIcon() const
{
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#3498db"));

    painter.drawRoundedRect(10, 40, 80, 48, 10, 10);
    painter.drawEllipse(QPointF(50, 64), 20, 20);
    painter.drawRect(30, 30, 40, 10);
    QPainterPath path;
    path.moveTo(90, 45);
    path.lineTo(118, 35);
    path.lineTo(118, 55);
    path.closeSubpath();
    painter.drawPath(path);

    return QIcon(pixmap);
}

QIcon SessionWidget::createJoinIcon() const
{
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor("#3498db"));
    pen.setWidth(10);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.drawArc(20, 44, 40, 40, 45 * 16, 270 * 16);
    painter.drawArc(68, 44, 40, 40, 225 * 16, 270 * 16);
    pen.setCapStyle(Qt::FlatCap);
    painter.setPen(pen);
    painter.drawLine(50, 48, 78, 48);
    painter.drawLine(50, 80, 78, 80);

    return QIcon(pixmap);
}

QIcon SessionWidget::createWrenchIcon() const
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::white);
    pen.setWidth(4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    painter.translate(32, 32);
    painter.rotate(45);

    painter.drawRoundedRect(-24, -4, 48, 8, 4, 4);
    painter.drawArc(-20, -24, 40, 40, 0 * 16, 270 * 16);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(QColor("#2c3e50"));
    painter.drawPath(QPainterPath());
    painter.drawArc(-12, -16, 24, 24, 0 * 16, 270 * 16);

    return QIcon(pixmap);
}

void SessionWidget::updateConnectionStatus(bool connected, const QString& text)
{
    ui->connectionStatusIndicator->setProperty("status", connected ? "connected" : "disconnected");
    ui->connectionStatusText->setText(text);
    ui->connectionStatusIndicator->style()->unpolish(ui->connectionStatusIndicator);
    ui->connectionStatusIndicator->style()->polish(ui->connectionStatusIndicator);
}

void SessionWidget::updateUiState() {
    const bool inOrder = !currentOrderId_.isEmpty();
    ui->orderEdit->setEnabled(!inOrder);
    ui->joinBtn->setEnabled(!inOrder);
    ui->leaveBtn->setEnabled(inOrder);
    ui->chatInputEdit->setEnabled(inOrder);
    ui->chatSendBtn->setEnabled(inOrder && !ui->chatInputEdit->toPlainText().trimmed().isEmpty());

    if (recordBtn_) {
        // 始终允许点击；录制逻辑在 onRecordButtonClicked 里判断 currentOrderId_
        recordBtn_->setEnabled(true);
        recordBtn_->setToolTip(inOrder ? QString() : tr("请先加入工单后再开始录制"));
        if (!inOrder && recorder_.isRecording()) {
            recorder_.stop();
        }
    }
}

void SessionWidget::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    QTimer::singleShot(0, this, [this]{
        qDebug() << "[rec btn after show] visible=" << (recordBtn_ ? recordBtn_->isVisible() : false)
                 << "enabled=" << (recordBtn_ ? recordBtn_->isEnabled() : false)
                 << "parent=" << (recordBtn_ && recordBtn_->parent() ? recordBtn_->parent()->metaObject()->className() : "null");
    });
}

void SessionWidget::onNavigationChanged()
{
    if (ui->navButton_DeviceData->isChecked()) ui->mainContentStack->setCurrentWidget(ui->page_DeviceData);
    else if (ui->navButton_JoinOrder->isChecked()) ui->mainContentStack->setCurrentWidget(ui->page_JoinOrder);
    else if (ui->navButton_History->isChecked()) {
        ui->mainContentStack->setCurrentWidget(ui->page_History);
        if(ticketsPage_) ticketsPage_->refresh();
    }
    else if (ui->navButton_Chat->isChecked()) ui->mainContentStack->setCurrentWidget(ui->page_Chat);
    else if (ui->navButton_Meeting->isChecked()) ui->mainContentStack->setCurrentWidget(ui->page_Meeting);
    else if (ui->navButton_Account->isChecked()) ui->mainContentStack->setCurrentWidget(ui->page_Account);
}

// 录制相关
void SessionWidget::updateRecordUi(bool recording) {
    if (!recordBtn_) return;
    recordBtn_->setProperty("recording", recording);
    if (recording) {
        recordBtn_->setText(tr("停止录制"));
    } else {
        recordSeconds_ = 0;
        recordBtn_->setText(tr("开始录制"));
    }
    recordBtn_->style()->unpolish(recordBtn_);
    recordBtn_->style()->polish(recordBtn_);
}

void SessionWidget::onRecordButtonClicked() {
    if (!recorder_.isRecording()) {
        if (currentOrderId_.isEmpty()) {
            QMessageBox::warning(this, tr("无法开始录制"), tr("请先加入工单"));
            return;
        }
        recordSeconds_ = 0;
        if (recorder_.start(currentOrderId_, /*overlayText=*/true, /*withAudio=*/true)) {
            updateRecordUi(true);
        }
    } else {
        recorder_.stop();
    }
}

void SessionWidget::onRecordStarted(const QString& file) {
    Q_UNUSED(file);
    updateRecordUi(true);
}

void SessionWidget::onRecordStopped(const QString& file) {
    Q_UNUSED(file);
    updateRecordUi(false);
}

void SessionWidget::onRecordError(const QString& msg) {
    QMessageBox::warning(this, tr("录制错误"), msg);
    updateRecordUi(false);
}

void SessionWidget::onRecordProgress(int sec) {
    recordSeconds_ = sec;
    if (recordBtn_) {
        recordBtn_->setText(tr("停止录制 (%1)").arg(fmtDuration(sec)));
    }
}
