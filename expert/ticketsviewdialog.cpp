#include "ticketsviewdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include "ticketsclient.h"
#include "workordermanager.h"

TicketsViewDialog::TicketsViewDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("历史工单（含日志/故障）"));
    resize(1100, 600);

    auto* root = new QVBoxLayout(this);

    // 顶部按钮
    auto* topRow = new QHBoxLayout();
    btnRefresh_ = new QPushButton(tr("刷新"), this);
    btnJoin_    = new QPushButton(tr("加入选中工单"), this);
    btnLogs_    = new QPushButton(tr("查看日志"), this);
    btnFaults_  = new QPushButton(tr("查看故障"), this);
    topRow->addWidget(btnRefresh_);
    topRow->addWidget(btnJoin_);
    topRow->addStretch();
    topRow->addWidget(btnLogs_);
    topRow->addWidget(btnFaults_);
    root->addLayout(topRow);

    // 中部：左侧工单列表 + 右侧日志/故障
    auto* split = new QSplitter(Qt::Horizontal, this);

    // 左：工单列表
    table_ = new QTableView(split);
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels(QStringList() << "编号" << "标题" << "优先级" << "状态" << "创建者" << "创建时间" << "日志数" << "故障数");
    table_->setModel(model_);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 右：日志 + 故障
    auto* right = new QWidget(split);
    auto* rightLy = new QVBoxLayout(right);

    auto* logBox = new QGroupBox(tr("设备日志"), right);
    auto* logLy = new QVBoxLayout(logBox);
    logsView_ = new QTableView(logBox);
    logsModel_ = new QStandardItemModel(this);
    logsModel_->setHorizontalHeaderLabels(QStringList() << "ID" << "温度" << "压力" << "振动" << "电流" << "电压" << "转速" << "状态" << "时间");
    logsView_->setModel(logsModel_);
    logsView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    logLy->addWidget(logsView_);
    rightLy->addWidget(logBox, 1);

    auto* faultBox = new QGroupBox(tr("故障信息"), right);
    auto* faultLy = new QVBoxLayout(faultBox);
    faultsView_ = new QTableView(faultBox);
    faultsModel_ = new QStandardItemModel(this);
    faultsModel_->setHorizontalHeaderLabels(QStringList() << "ID" << "代码" << "描述" << "等级" << "时间");
    faultsView_->setModel(faultsModel_);
    faultsView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    faultLy->addWidget(faultsView_);
    rightLy->addWidget(faultBox, 1);

    split->addWidget(table_);
    split->addWidget(right);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 5);
    root->addWidget(split, 1);

    // 事件
    connect(btnRefresh_, &QPushButton::clicked, this, &TicketsViewDialog::refresh);
    connect(btnJoin_,    &QPushButton::clicked, this, &TicketsViewDialog::onJoinSelected);
    connect(btnLogs_,    &QPushButton::clicked, this, &TicketsViewDialog::onLoadLogs);
    connect(btnFaults_,  &QPushButton::clicked, this, &TicketsViewDialog::onLoadFaults);
}

void TicketsViewDialog::bindServices(TicketsClient* cli, WorkOrderManager* orders) {
    cli_ = cli; orders_ = orders;
    if (cli_) {
        connect(cli_, &TicketsClient::listed,      this, &TicketsViewDialog::onListed);
        connect(cli_, &TicketsClient::listFailed,  this, &TicketsViewDialog::onListFailed);
        connect(cli_, &TicketsClient::logsListed,  this, &TicketsViewDialog::onLogsListed);
        connect(cli_, &TicketsClient::logsFailed,  this, &TicketsViewDialog::onLogsFailed);
        connect(cli_, &TicketsClient::faultsListed,this, &TicketsViewDialog::onFaultsListed);
        connect(cli_, &TicketsClient::faultsFailed,this, &TicketsViewDialog::onFaultsFailed);
    }
}

void TicketsViewDialog::refresh() {
    if (cli_) cli_->list(1, 100);
}

int TicketsViewDialog::selectedId() const {
    const auto idx = table_->currentIndex();
    if (!idx.isValid()) return -1;
    return model_->item(idx.row(), 0)->text().toInt();
}

void TicketsViewDialog::onJoinSelected() {
    if (!orders_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选中一条工单")); return; }
    orders_->join(QString::number(id));
}

void TicketsViewDialog::onListed(const QJsonArray& rows) {
    model_->removeRows(0, model_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(o.value("id").toInt()));
        row << new QStandardItem(o.value("title").toString());
        row << new QStandardItem(o.value("priority").toString());
        row << new QStandardItem(QString::number(o.value("status").toInt()));
        row << new QStandardItem(o.value("creator").toString());
        row << new QStandardItem(o.value("created_time").toString());
        row << new QStandardItem(QString::number(o.value("logs").toInt()));    // 日志数
        row << new QStandardItem(QString::number(o.value("faults").toInt()));  // 故障数
        model_->appendRow(row);
    }
}

void TicketsViewDialog::onListFailed(const QString& reason) {
    QMessageBox::warning(this, tr("加载失败"), reason);
}

void TicketsViewDialog::onLoadLogs()   { if (cli_) cli_->logs(selectedId(), 1, 100); }
void TicketsViewDialog::onLoadFaults() { if (cli_) cli_->faults(selectedId(), 1, 100); }

void TicketsViewDialog::onLogsListed(int ticketId, const QJsonArray& rows) {
    Q_UNUSED(ticketId);
    logsModel_->removeRows(0, logsModel_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        QList<QStandardItem*> r;
        r << new QStandardItem(QString::number(o.value("id").toInt()));
        r << new QStandardItem(QString::number(o.value("temperature").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("pressure").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("vibration").toDouble(), 'f', 2));
        r << new QStandardItem(QString::number(o.value("current").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("voltage").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("speed").toInt()));
        r << new QStandardItem(o.value("status").toInt() ? "1":"0");
        r << new QStandardItem(o.value("record_time").toString());
        logsModel_->appendRow(r);
    }
}

void TicketsViewDialog::onLogsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("日志加载失败"), reason);
}

void TicketsViewDialog::onFaultsListed(int ticketId, const QJsonArray& rows) {
    Q_UNUSED(ticketId);
    faultsModel_->removeRows(0, faultsModel_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        QList<QStandardItem*> r;
        r << new QStandardItem(QString::number(o.value("id").toInt()));
        r << new QStandardItem(o.value("code").toString());
        r << new QStandardItem(o.value("text").toString());
        r << new QStandardItem(o.value("level").toString());
        r << new QStandardItem(o.value("ts").toString());
        faultsModel_->appendRow(r);
    }
}

void TicketsViewDialog::onFaultsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("故障加载失败"), reason);
}
