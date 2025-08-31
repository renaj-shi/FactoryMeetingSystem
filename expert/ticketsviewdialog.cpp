#include "ticketsviewdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include "ticketsclient.h"
#include "workordermanager.h"
#include <QJsonArray>
#include <QJsonObject>

TicketsViewDialog::TicketsViewDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("历史工单"));
    resize(900, 520);

    auto* root = new QVBoxLayout(this);
    table_ = new QTableView(this);
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels(QStringList() << "编号" << "标题" << "优先级" << "状态" << "创建者" << "指派专家" << "创建时间");
    table_->setModel(model_);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(table_, 1);

    auto* row = new QHBoxLayout();
    row->addStretch();
    btnRefresh_ = new QPushButton(tr("刷新"), this);
    btnJoin_    = new QPushButton(tr("加入选中工单"), this);
    row->addWidget(btnRefresh_);
    row->addWidget(btnJoin_);
    root->addLayout(row);

    connect(btnRefresh_, &QPushButton::clicked, this, &TicketsViewDialog::onRefresh);
    connect(btnJoin_,    &QPushButton::clicked, this, &TicketsViewDialog::onJoinSelected);
}

void TicketsViewDialog::bindServices(TicketsClient* cli, WorkOrderManager* orders) {
    cli_ = cli; orders_ = orders;
    if (cli_) {
        connect(cli_, &TicketsClient::listed,     this, &TicketsViewDialog::onListed);
        connect(cli_, &TicketsClient::listFailed, this, &TicketsViewDialog::onListFailed);
    }
}

void TicketsViewDialog::refresh() {
    if (cli_) cli_->list(1, 100);
}

void TicketsViewDialog::onRefresh() { refresh(); }

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
        // 状态：0待处理/1处理中/2完成（后端自定）
        const int st = o.value("status").toInt();
        row << new QStandardItem(QString::number(st));
        row << new QStandardItem(o.value("creator").toString());
        row << new QStandardItem(o.value("expert_username").toVariant().toString());
        row << new QStandardItem(o.value("created_time").toString());
        model_->appendRow(row);
    }
}

void TicketsViewDialog::onListFailed(const QString& reason) {
    QMessageBox::warning(this, tr("加载失败"), reason);
}
