#include "sessionwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

#include "tcpclient.h"
#include "authmanager.h"
#include "workordermanager.h"
#include "telemetryclient.h"

#include "chatclient.h"

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
using namespace QtCharts;

SessionWidget::SessionWidget(QWidget* parent) : QWidget(parent) {
    buildUi();
}

void SessionWidget::buildUi() {
    auto* root = new QVBoxLayout(this);

    // 工单栏
    auto* orderBox = new QGroupBox(tr("工单"), this);
    auto* orderLy  = new QHBoxLayout(orderBox);
    orderEdit_     = new QLineEdit(orderBox); orderEdit_->setPlaceholderText(tr("输入工单号..."));
    joinBtn_       = new QPushButton(tr("加入"), orderBox);
    leaveBtn_      = new QPushButton(tr("离开"), orderBox);
    orderStatusLabel_ = new QLabel(tr("未加入"), orderBox);
    orderStatusLabel_->setStyleSheet("color:#888;");
    orderLy->addWidget(new QLabel(tr("工单号:"), orderBox));
    orderLy->addWidget(orderEdit_, 1);
    orderLy->addWidget(joinBtn_);
    orderLy->addWidget(leaveBtn_);
    orderLy->addWidget(orderStatusLabel_);
    root->addWidget(orderBox);

    // 中间：设备曲线 + 右侧：日志/故障
    auto* mid = new QHBoxLayout();
    buildCharts();
    mid->addWidget(chartView_, 2);

    auto* right = new QVBoxLayout();
    auto* logBox = new QGroupBox(tr("设备日志"), this);
    auto* logLy  = new QVBoxLayout(logBox);
    logView_     = new QPlainTextEdit(logBox); logView_->setReadOnly(true);
    logLy->addWidget(logView_);
    right->addWidget(logBox, 1);

    auto* faultBox = new QGroupBox(tr("故障信息"), this);
    auto* faultLy  = new QVBoxLayout(faultBox);
    faultView_     = new QPlainTextEdit(faultBox); faultView_->setReadOnly(true);
    faultLy->addWidget(faultView_);
    right->addWidget(faultBox, 1);

    mid->addLayout(right, 1);
    root->addLayout(mid, 3);

    // 下方：聊天（暂本地回显）
    auto* chatBox = new QGroupBox(tr("聊天"), this);
    auto* cLy = new QVBoxLayout(chatBox);
    chatView_ = new QPlainTextEdit(chatBox); chatView_->setReadOnly(true);
    auto* row = new QHBoxLayout();
    chatEdit_ = new QLineEdit(chatBox); chatEdit_->setPlaceholderText(tr("输入消息..."));
    chatSendBtn_ = new QPushButton(tr("发送"), chatBox);
    row->addWidget(chatEdit_, 1); row->addWidget(chatSendBtn_);
    cLy->addWidget(chatView_, 1); cLy->addLayout(row);
    root->addWidget(chatBox, 1);

    // 事件
    connect(joinBtn_,  &QPushButton::clicked, this, &SessionWidget::onJoinClicked);
    connect(leaveBtn_, &QPushButton::clicked, this, &SessionWidget::onLeaveClicked);
    connect(chatSendBtn_, &QPushButton::clicked, this, &SessionWidget::onSendChat);

    updateUiState();
}

void SessionWidget::buildCharts() {
    tempSeries_  = new QLineSeries(this); tempSeries_->setName(tr("温度(°C)"));
    pressSeries_ = new QLineSeries(this); pressSeries_->setName(tr("压力(kPa)"));

    chart_ = new QChart();
    chart_->addSeries(tempSeries_);
    chart_->addSeries(pressSeries_);
    chart_->setTitle(tr("温度/压力 实时曲线"));
    chart_->legend()->setAlignment(Qt::AlignBottom);

    axisX_ = new QDateTimeAxis(this);
    axisX_->setFormat("HH:mm:ss");
    axisX_->setTitleText(tr("时间"));

    axisY_ = new QValueAxis(this);
    axisY_->setTitleText(tr("数值"));
    axisY_->setLabelFormat("%.1f");

    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);
    tempSeries_->attachAxis(axisX_); tempSeries_->attachAxis(axisY_);
    pressSeries_->attachAxis(axisX_); pressSeries_->attachAxis(axisY_);

    chartView_ = new QChartView(chart_);
    chartView_->setRenderHint(QPainter::Antialiasing);
}

void SessionWidget::bindServices(TcpClient* tcp, AuthManager* auth, WorkOrderManager* orders, TelemetryClient* telem) {
    tcp_ = tcp; auth_ = auth; orders_ = orders; telem_ = telem;

    if (orders_) {
        connect(orders_, &WorkOrderManager::joined,     this, &SessionWidget::onJoined);
        connect(orders_, &WorkOrderManager::joinFailed, this, &SessionWidget::onJoinFailed);
        connect(orders_, &WorkOrderManager::left,       this, &SessionWidget::onLeft);
    }
    if (telem_) {
        connect(telem_, &TelemetryClient::metricsUpdated, this, &SessionWidget::onMetrics);
        connect(telem_, &TelemetryClient::logAppended,    this, &SessionWidget::onLogLine);
        connect(telem_, &TelemetryClient::faultRaised,     this, &SessionWidget::onFault);
    }
}

void SessionWidget::setOrderId(const QString& id) {
    currentOrderId_ = id;
    orderStatusLabel_->setText(id.isEmpty() ? tr("未加入") : tr("已加入: %1").arg(id));
    updateUiState();
}

void SessionWidget::onJoinClicked() {
    if (!orders_) return;
    const QString oid = orderEdit_->text().trimmed();
    if (oid.isEmpty()) return;
    orders_->join(oid);
}

void SessionWidget::onLeaveClicked() {
    if (!orders_) return;
    orders_->leave();
    if (telem_) telem_->unsubscribe();
}

void SessionWidget::onJoined(const QString& orderId) {
    setOrderId(orderId);
    if (telem_) telem_->subscribe(orderId);
    appendChatLine(QString("系统: 已加入工单 %1").arg(orderId));
}

void SessionWidget::onJoinFailed(const QString& reason) {
    appendChatLine(QString("系统: 加入失败 - %1").arg(reason));
}

void SessionWidget::onLeft(const QString& orderId) {
    Q_UNUSED(orderId);
    setOrderId(QString());
    if (telem_) telem_->unsubscribe();
    appendChatLine("系统: 已离开工单");
}

void SessionWidget::onMetrics(const QString& orderId, const QJsonObject& m) {
    if (orderId != currentOrderId_) return;
    // 取 ts/temperature/pressure
    const qint64 ts = m.value("ts").toVariant().toLongLong();
    const double t  = m.value("temperature").toDouble();
    const double p  = m.value("pressure").toDouble();

    appendPoint(ts, t, p);

    // 可选：也把其它字段追加到日志
    if (m.contains("vibration"))
        appendLogLine(tr("振动: %1 mm/s").arg(m.value("vibration").toDouble(), 0, 'f', 2));
    if (m.contains("status"))
        appendLogLine(tr("状态: %1").arg(m.value("status").toBool() ? tr("运行中") : tr("停止")));
}

void SessionWidget::appendPoint(qint64 ms, double temp, double press) {
    const qreal x = ms; // QDateTimeAxis 使用毫秒时间戳
    tempSeries_->append(x, temp);
    pressSeries_->append(x, press);

    // 限制点数
    if (tempSeries_->count() > maxPoints_)
        tempSeries_->removePoints(0, tempSeries_->count() - maxPoints_);
    if (pressSeries_->count() > maxPoints_)
        pressSeries_->removePoints(0, pressSeries_->count() - maxPoints_);

    // 更新坐标轴范围（窗口滑动）
    if (tempSeries_->count() > 1) {
        const qint64 xmin = tempSeries_->at(0).x();
        const qint64 xmax = tempSeries_->at(tempSeries_->count()-1).x();
        axisX_->setRange(QDateTime::fromMSecsSinceEpoch(xmin),
                         QDateTime::fromMSecsSinceEpoch(xmax));
        // 根据最近点自动调 Y
        double ymin = 1e9, ymax = -1e9;
        for (const auto& p : tempSeries_->pointsVector()) { ymin = qMin(ymin, p.y()); ymax = qMax(ymax, p.y()); }
        for (const auto& p : pressSeries_->pointsVector()) { ymin = qMin(ymin, p.y()); ymax = qMax(ymax, p.y()); }
        if (ymin < ymax) axisY_->setRange(ymin - 1, ymax + 1);
    }
}

void SessionWidget::onLogLine(const QString& orderId, const QString& line) {
    if (orderId != currentOrderId_) return;
    appendLogLine(line);
}

void SessionWidget::onFault(const QString& orderId, const QString& code, const QString& text, const QString& level) {
    if (orderId != currentOrderId_) return;
    appendFaultLine(tr("[%1] %2: %3").arg(level, code, text));
}

// 新增：挂接聊天服务
void SessionWidget::attachChat(ChatClient* chat) {
    chat_ = chat;
    if (chat_) {
        connect(chat_, &ChatClient::messageReceived,
                this, &SessionWidget::onChatMessage);
    }
}

// 修改：发送聊天
void SessionWidget::onSendChat() {
    const QString text = chatEdit_->text().trimmed();
    if (text.isEmpty() || currentOrderId_.isEmpty()) return;

    if (chat_) {
        chat_->sendText(currentOrderId_, text);
        chatEdit_->clear();
        // 不做本地回显，等待服务端广播 CHAT|MSG 后统一显示
    } else {
        // 兜底：若未注入聊天模块，仍保持本地回显（可选）
        appendChatLine(QString("[%1] 我: %2")
                           .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), text));
        chatEdit_->clear();
    }
}

// 新增：收到服务器广播的消息
void SessionWidget::onChatMessage(const QString& orderId, const QString& from, const QString& text, qint64 ts) {
    if (orderId != currentOrderId_) return;
    const auto t = QDateTime::fromMSecsSinceEpoch(ts).toString("HH:mm:ss");
    appendChatLine(QString("[%1] %2: %3").arg(t, from, text));
}

void SessionWidget::appendChatLine(const QString& line)  { chatView_->appendPlainText(line); }
void SessionWidget::appendLogLine(const QString& line)   { logView_->appendPlainText(line); }
void SessionWidget::appendFaultLine(const QString& line) { faultView_->appendPlainText(line); }

void SessionWidget::updateUiState() {
    const bool inOrder = !currentOrderId_.isEmpty();
    orderEdit_->setEnabled(!inOrder);
    joinBtn_->setEnabled(!inOrder);
    leaveBtn_->setEnabled(inOrder);
}
