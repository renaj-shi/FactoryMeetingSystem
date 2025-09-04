#include "knowledgeviewdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include "knowledgeclient.h"
#include <QJsonArray>
#include <QJsonObject>

KnowledgeViewDialog::KnowledgeViewDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("企业知识库"));
    resize(1000, 560);

    auto* root = new QVBoxLayout(this);

    table_ = new QTableView(this);
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels(QStringList() << "编号" << "标题" << "创建时间" << "温度" << "压力" << "振动" << "电流" << "电压" << "转速");
    table_->setModel(model_);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(table_, 2);

    detail_ = new QPlainTextEdit(this);
    detail_->setReadOnly(true);
    root->addWidget(detail_, 1);

    auto* row = new QHBoxLayout();
    row->addStretch();
    btnRefresh_ = new QPushButton(tr("刷新"), this);
    btnDetail_  = new QPushButton(tr("查看详情JSON"), this);
    row->addWidget(btnRefresh_);
    row->addWidget(btnDetail_);
    root->addLayout(row);

    connect(btnRefresh_, &QPushButton::clicked, this, &KnowledgeViewDialog::refresh);
    connect(btnDetail_,  &QPushButton::clicked, this, &KnowledgeViewDialog::onViewDetail);
}

void KnowledgeViewDialog::bindServices(KnowledgeClient* cli) {
    cli_ = cli;
    if (cli_) {
        connect(cli_, &KnowledgeClient::listed,     this, &KnowledgeViewDialog::onListed);
        connect(cli_, &KnowledgeClient::listFailed, this, &KnowledgeViewDialog::onListFailed);
        // 可选：connect(cli_, &KnowledgeClient::got, ... 显示详情)
    }
}

void KnowledgeViewDialog::refresh() {
    if (cli_) cli_->list(1, 100);
}

int KnowledgeViewDialog::selectedId() const {
    const auto idx = table_->currentIndex();
    if (!idx.isValid()) return -1;
    return model_->item(idx.row(), 0)->text().toInt();
}

void KnowledgeViewDialog::onListed(const QJsonArray& rows) {
    model_->removeRows(0, model_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        const auto dp = o.value("device_params").toObject();
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(o.value("id").toInt()));
        row << new QStandardItem(o.value("title").toString());
        row << new QStandardItem(o.value("created_time").toString());
        row << new QStandardItem(QString::number(dp.value("temperature").toDouble(), 'f', 1));
        row << new QStandardItem(QString::number(dp.value("pressure").toDouble(), 'f', 1));
        row << new QStandardItem(QString::number(dp.value("vibration").toDouble(), 'f', 2));
        row << new QStandardItem(QString::number(dp.value("current").toDouble(), 'f', 1));
        row << new QStandardItem(QString::number(dp.value("voltage").toDouble(), 'f', 1));
        row << new QStandardItem(QString::number(dp.value("speed").toInt()));
        model_->appendRow(row);
    }
}

void KnowledgeViewDialog::onListFailed(const QString& reason) {
    QMessageBox::warning(this, tr("加载失败"), reason);
}

void KnowledgeViewDialog::onViewDetail() {
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请选择一条记录")); return; }
    // 直接从当前行组装 JSON 展示（也可调用 cli_->get(id) 取更完整详情）
    const int row = table_->currentIndex().row();
    QJsonObject dp;
    dp["temperature"] = model_->item(row,3)->text().toDouble();
    dp["pressure"]    = model_->item(row,4)->text().toDouble();
    dp["vibration"]   = model_->item(row,5)->text().toDouble();
    dp["current"]     = model_->item(row,6)->text().toDouble();
    dp["voltage"]     = model_->item(row,7)->text().toDouble();
    dp["speed"]       = model_->item(row,8)->text().toInt();

    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = model_->item(row,1)->text();
    obj["created_time"] = model_->item(row,2)->text();
    obj["device_params"] = dp;

    detail_->setPlainText(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented)));
}
