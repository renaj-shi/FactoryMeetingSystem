// ===================================================================
// [REVERT] 文件名: ticketspagewidget.cpp
// [REVERT] 内容: 最终完整版。此版本已将“定位到日志”功能回退到原始的、仅在当前页面搜索的逻辑。
// [REVERT]       它包含了所有UI优化和之前的功能修正。
// ===================================================================
#include "ticketspagewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QJsonObject>
#include <QFormLayout>
#include <QDebug>

#include "ticketsclient.h"
#include "workordermanager.h"

// [MOD] 再次简化了工单列表的列定义，只保留ID和标题
static constexpr int COL_ID       = 0;
static constexpr int COL_TITLE    = 1;
// [DEL] 以下列定义已完全从主列表中移除
// static constexpr int COL_STATUS   = 2;
// static constexpr int COL_CTIME    = 3;

// 日志列索引（序号、温度、压力、振动、电流、电压、转速、状态、时间、ID）
static constexpr int LCOL_SEQ   = 0;
static constexpr int LCOL_TEMP  = 1;
static constexpr int LCOL_PRES  = 2;
static constexpr int LCOL_VIB   = 3;
static constexpr int LCOL_CURR  = 4;
static constexpr int LCOL_VOLT  = 5;
static constexpr int LCOL_SPEED = 6;
static constexpr int LCOL_STAT  = 7;
static constexpr int LCOL_TIME  = 8;
static constexpr int LCOL_ID    = 9; // 隐藏（数据库 id）

// 故障列索引（序号、代码、描述、时间、ID）
static constexpr int FCOL_SEQ   = 0;
static constexpr int FCOL_CODE  = 1;
static constexpr int FCOL_TEXT  = 2;
static constexpr int FCOL_TIME  = 3;
static constexpr int FCOL_ID    = 4; // 隐藏（数据库 id）

TicketsPageWidget::TicketsPageWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);

    auto* topRow = new QHBoxLayout();
    btnRefresh_ = new QPushButton(tr("刷新"), this);
    btnJoin_    = new QPushButton(tr("再次加入所选工单"), this);
    topRow->addWidget(btnRefresh_);
    topRow->addWidget(btnJoin_);
    topRow->addStretch();
    root->addLayout(topRow);

    auto* split = new QSplitter(Qt::Horizontal, this);

    // 左：工单列表
    table_ = new QTableView(split);
    model_ = new QStandardItemModel(this);
    // [MOD] 设置最终简化的表头，只剩两列
    model_->setHorizontalHeaderLabels(QStringList() << tr("编号") << tr("标题"));
    table_->setModel(model_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    // [MOD] 让标题列占据绝大部分空间，编号列根据内容自适应
    table_->horizontalHeader()->setSectionResizeMode(COL_ID, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(COL_TITLE, QHeaderView::Stretch);
    // [ADD] 本次修改新增：开启toolTip，当鼠标悬停在单元格上时显示完整内容
    table_->setMouseTracking(true);
    connect(table_, &QTableView::entered, [this](const QModelIndex &index){
        if (index.isValid()) {
            table_->setToolTip(index.data().toString());
        } else {
            table_->setToolTip("");
        }
    });


    // 右：详情 + 日志 + 故障
    auto* right = new QWidget(split);
    auto* rightLy = new QVBoxLayout(right);

    // [MOD] 本次修改核心：重构工单详情面板的布局
    auto* detailBox = new QGroupBox(tr("工单详情"), right);
    auto* detailVBox = new QVBoxLayout(detailBox); // 使用垂直布局作为基础

    // [MOD] 第一行：状态 + 创建时间
    auto* row1 = new QHBoxLayout();
    row1->addWidget(new QLabel(tr("状态:"), detailBox));
    detailStatusLabel_ = new QLabel("-", detailBox);
    row1->addWidget(detailStatusLabel_, 1); // 添加伸展因子
    row1->addWidget(new QLabel(tr("创建时间:"), detailBox));
    detailTimeLabel_ = new QLabel("-", detailBox);
    row1->addWidget(detailTimeLabel_, 2); // 添加伸展因子
    detailVBox->addLayout(row1);

    // [MOD] 第二行：优先级 + 创建者
    auto* row2 = new QHBoxLayout();
    row2->addWidget(new QLabel(tr("优先级:"), detailBox));
    detailPriorityLabel_ = new QLabel("-", detailBox);
    row2->addWidget(detailPriorityLabel_, 1);
    row2->addWidget(new QLabel(tr("创建者:"), detailBox));
    detailCreatorLabel_ = new QLabel("-", detailBox);
    row2->addWidget(detailCreatorLabel_, 2);
    detailVBox->addLayout(row2);

    // [MOD] 第三行：日志数 + 故障数
    auto* row3 = new QHBoxLayout();
    row3->addWidget(new QLabel(tr("日志数:"), detailBox));
    detailLogsCountLabel_ = new QLabel("-", detailBox);
    row3->addWidget(detailLogsCountLabel_, 1);
    row3->addWidget(new QLabel(tr("故障数:"), detailBox));
    detailFaultsCountLabel_ = new QLabel("-", detailBox);
    row3->addWidget(detailFaultsCountLabel_, 2);
    detailVBox->addLayout(row3);

    rightLy->addWidget(detailBox);

    // 日志
    auto* logBox = new QGroupBox(tr("设备日志"), right);
    auto* logLy  = new QVBoxLayout(logBox);
    logsView_    = new QTableView(logBox);
    logsModel_   = new QStandardItemModel(this);
    logsModel_->setHorizontalHeaderLabels(QStringList()
                                          << tr("序号") << tr("温度") << tr("压力") << tr("振动")
                                          << tr("电流") << tr("电压") << tr("转速") << tr("状态") << tr("时间") << tr("ID"));
    logsView_->setModel(logsModel_);
    logsView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    logsView_->setColumnHidden(LCOL_ID, true);
    // [ADD] 本次修改新增：为日志列表也开启toolTip功能
    logsView_->setMouseTracking(true);
    connect(logsView_, &QTableView::entered, [this](const QModelIndex &index){
        if (index.isValid()) {
            logsView_->setToolTip(index.data().toString());
        } else {
            logsView_->setToolTip("");
        }
    });

    auto* logsCtrl = new QHBoxLayout();
    btnLogs_        = new QPushButton(tr("查看日志"), logBox);
    btnExportLogs_  = new QPushButton(tr("导出CSV"), logBox);
    logsPrev_       = new QPushButton(tr("上一页"), logBox);
    logsNext_       = new QPushButton(tr("下一页"), logBox);
    logsPage_       = new QSpinBox(logBox); logsPage_->setRange(1, 1000000); logsPage_->setValue(1);
    logsPageSize_   = new QSpinBox(logBox); logsPageSize_->setRange(50, 10000); logsPageSize_->setValue(200);

    logsCtrl->addWidget(new QLabel(tr("页:")));
    logsCtrl->addWidget(logsPage_);
    logsCtrl->addWidget(new QLabel(tr("每页:")));
    logsCtrl->addWidget(logsPageSize_);
    logsCtrl->addWidget(logsPrev_);
    logsCtrl->addWidget(logsNext_);
    logsCtrl->addStretch();
    logsCtrl->addWidget(btnLogs_);
    logsCtrl->addWidget(btnExportLogs_);

    logLy->addWidget(logsView_);
    logLy->addLayout(logsCtrl);
    rightLy->addWidget(logBox, 1);

    // 故障
    auto* faultBox = new QGroupBox(tr("故障信息"), right);
    auto* faultLy  = new QVBoxLayout(faultBox);
    faultsView_    = new QTableView(faultBox);
    faultsModel_   = new QStandardItemModel(this);
    faultsModel_->setHorizontalHeaderLabels(QStringList()
                                            << tr("序号") << tr("代码") << tr("描述") << tr("时间") << tr("ID"));
    faultsView_->setModel(faultsModel_);
    faultsView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    faultsView_->setColumnHidden(FCOL_ID, true);

    auto* faultsCtrl = new QHBoxLayout();
    btnFaults_        = new QPushButton(tr("查看故障"), faultBox);
    btnExportFaults_  = new QPushButton(tr("导出CSV"), faultBox);
    btnLocate_        = new QPushButton(tr("定位到日志"), faultBox);
    faultsPrev_       = new QPushButton(tr("上一页"), faultBox);
    faultsNext_       = new QPushButton(tr("下一页"), faultBox);
    faultsPage_       = new QSpinBox(faultBox); faultsPage_->setRange(1, 1000000); faultsPage_->setValue(1);
    faultsPageSize_   = new QSpinBox(faultBox); faultsPageSize_->setRange(50, 10000); faultsPageSize_->setValue(200);

    faultsCtrl->addWidget(new QLabel(tr("页:")));
    faultsCtrl->addWidget(faultsPage_);
    faultsCtrl->addWidget(new QLabel(tr("每页:")));
    faultsCtrl->addWidget(faultsPageSize_);
    faultsCtrl->addWidget(faultsPrev_);
    faultsCtrl->addWidget(faultsNext_);
    faultsCtrl->addStretch();
    faultsCtrl->addWidget(btnFaults_);
    faultsCtrl->addWidget(btnExportFaults_);
    faultsCtrl->addWidget(btnLocate_);

    faultLy->addWidget(faultsView_);
    faultLy->addLayout(faultsCtrl);
    rightLy->addWidget(faultBox, 1);

    split->addWidget(table_);
    split->addWidget(right);
    // [MOD] 调整分栏比例，给左侧列表更合理的空间，例如 1:2
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 2);
    root->addWidget(split, 1);

    // 事件绑定
    connect(btnRefresh_, &QPushButton::clicked, this, &TicketsPageWidget::refresh);
    connect(btnJoin_,    &QPushButton::clicked, this, &TicketsPageWidget::onJoinSelected);

    connect(btnLogs_,        &QPushButton::clicked, this, &TicketsPageWidget::onLoadLogs);
    connect(btnExportLogs_,  &QPushButton::clicked, this, &TicketsPageWidget::onExportLogs);
    connect(logsPrev_,       &QPushButton::clicked, this, [this]{ logsPage_->setValue(qMax(1, logsPage_->value()-1)); onLoadLogs(); });
    connect(logsNext_,       &QPushButton::clicked, this, [this]{ logsPage_->setValue(logsPage_->value()+1); onLoadLogs(); });

    connect(btnFaults_,        &QPushButton::clicked, this, &TicketsPageWidget::onLoadFaults);
    connect(btnExportFaults_,  &QPushButton::clicked, this, &TicketsPageWidget::onExportFaults);
    connect(btnLocate_,        &QPushButton::clicked, this, &TicketsPageWidget::onLocateFromFault);
    connect(faultsPrev_,       &QPushButton::clicked, this, [this]{ faultsPage_->setValue(qMax(1, faultsPage_->value()-1)); onLoadFaults(); });
    connect(faultsNext_,       &QPushButton::clicked, this, [this]{ faultsPage_->setValue(faultsPage_->value()+1); onLoadFaults(); });

    connect(table_->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &TicketsPageWidget::onSelectionChanged);
}

void TicketsPageWidget::bindServices(TicketsClient* cli, WorkOrderManager* orders) {
    cli_ = cli; orders_ = orders;
    if (cli_) {
        connect(cli_, &TicketsClient::listed,       this, &TicketsPageWidget::onListed);
        connect(cli_, &TicketsClient::listFailed,   this, &TicketsPageWidget::onListFailed);
        connect(cli_, &TicketsClient::logsListed,   this, &TicketsPageWidget::onLogsListed);
        connect(cli_, &TicketsClient::logsFailed,   this, &TicketsPageWidget::onLogsFailed);
        connect(cli_, &TicketsClient::faultsListed, this, &TicketsPageWidget::onFaultsListed);
        connect(cli_, &TicketsClient::faultsFailed, this, &TicketsPageWidget::onFaultsFailed);
        // [DEL] 移除了对已删除信号的 connect
        // connect(cli_, &TicketsClient::logLocated, this, &TicketsPageWidget::onLogLocated);
        // connect(cli_, &TicketsClient::locateLogFailed, this, &TicketsPageWidget::onLocateLogFailed);
    }
}

void TicketsPageWidget::refresh() {
    if (cli_) cli_->list(1, 200);
}

int TicketsPageWidget::selectedId() const {
    const auto idx = table_->currentIndex();
    if (!idx.isValid()) return -1;
    return model_->item(idx.row(), COL_ID)->text().toInt();
}

void TicketsPageWidget::onJoinSelected() {
    if (!orders_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选中一条工单")); return; }
    orders_->join(QString::number(id));
}

void TicketsPageWidget::onListed(const QJsonArray& rows) {
    // [DEBUG] 探针 #A (来自上次调试): 检查 onListed 槽是否被调用
    qDebug() << "[TicketsPageWidget] onListed slot called with array size:" << rows.size();

    model_->removeRows(0, model_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(o.value("id").toInt()));
        row << new QStandardItem(o.value("title").toString());
        model_->appendRow(row);

        QJsonObject fullData;
        fullData["status"] = o.value("status").toInt();
        fullData["created_time"] = o.value("created_time").toString();
        fullData["priority"] = o.value("priority").toString();
        fullData["creator"] = o.value("creator").toString();
        fullData["logs"] = o.value("logs").toInt();
        fullData["faults"] = o.value("faults").toInt();

        // [FIX] 本次修改核心：直接使用刚创建的item指针(row.at(COL_ID))，而不是从model中查找。
        row.at(COL_ID)->setData(QVariant(fullData), Qt::UserRole);
    }

    qDebug() << "[TicketsPageWidget] Model population finished. Final row count:" << model_->rowCount();

    if (model_->rowCount() > 0) {
        table_->selectRow(0);
    }
    onSelectionChanged();
}

void TicketsPageWidget::onListFailed(const QString& reason) {
    QMessageBox::warning(this, tr("加载失败"), reason);
}

void TicketsPageWidget::onLoadLogs() {
    if (!cli_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选择工单")); return; }
    currentTicketId_ = id;
    cli_->logs(id, logsPage_->value(), logsPageSize_->value());
}

void TicketsPageWidget::onLoadFaults() {
    if (!cli_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选择工单")); return; }
    currentTicketId_ = id;
    cli_->faults(id, faultsPage_->value(), faultsPageSize_->value());
}

void TicketsPageWidget::onLogsListed(int ticketId, const QJsonArray& rows) {
    Q_UNUSED(ticketId);
    logsModel_->removeRows(0, logsModel_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        const int seq = o.value("seq").toInt();
        const int id  = o.value("id").toInt();

        QList<QStandardItem*> r;
        r << new QStandardItem(QString::number(seq));
        r << new QStandardItem(QString::number(o.value("temperature").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("pressure").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("vibration").toDouble(), 'f', 2));
        r << new QStandardItem(QString::number(o.value("current").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("voltage").toDouble(), 'f', 1));
        r << new QStandardItem(QString::number(o.value("speed").toInt()));
        r << new QStandardItem(o.value("status").toInt() ? "1":"0");
        r << new QStandardItem(o.value("record_time").toString());
        r << new QStandardItem(QString::number(id));
        logsModel_->appendRow(r);
    }
    logsView_->setColumnHidden(LCOL_ID, true);

    // [DEL] 移除了检查异步定位标记的逻辑
    // if (!locateTsAfterLoad_.isEmpty()) {
    //     ...
    //     locateTsAfterLoad_.clear();
    // }
}

void TicketsPageWidget::onLogsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("日志加载失败"), reason);
}

void TicketsPageWidget::onFaultsListed(int ticketId, const QJsonArray& rows) {
    Q_UNUSED(ticketId);
    faultsModel_->removeRows(0, faultsModel_->rowCount());
    for (const auto& v : rows) {
        const auto o   = v.toObject();
        const int seq  = o.value("seq").toInt();
        const int id   = o.value("id").toInt();

        QList<QStandardItem*> r;
        r << new QStandardItem(QString::number(seq));
        r << new QStandardItem(o.value("code").toString());
        r << new QStandardItem(o.value("text").toString());
        r << new QStandardItem(o.value("ts").toString());
        r << new QStandardItem(QString::number(id));
        faultsModel_->appendRow(r);
    }
    faultsView_->setColumnHidden(FCOL_ID, true);
}

void TicketsPageWidget::onFaultsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("故障加载失败"), reason);
}

// [DEL] 移除了 onLogLocated 槽的实现
// void TicketsPageWidget::onLogLocated(int page, const QString& ts) { ... }

// [DEL] 移除了 onLocateLogFailed 槽的实现
// void TicketsPageWidget::onLocateLogFailed(const QString& reason) { ... }

void TicketsPageWidget::onExportLogs() {
    const QString path = QFileDialog::getSaveFileName(this, tr("导出日志为 CSV"),
                                                      tr("logs_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    exportModelToCsv(logsModel_, path);
}

void TicketsPageWidget::onExportFaults() {
    const QString path = QFileDialog::getSaveFileName(this, tr("导出故障为 CSV"),
                                                      tr("faults_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    exportModelToCsv(faultsModel_, path);
}

void TicketsPageWidget::onLocateFromFault() {
    // [REVERT] 恢复为原始的、仅在当前页面搜索的逻辑
    const auto idx = faultsView_->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("提示"), tr("请先选中一条故障"));
        return;
    }
    const int row = idx.row();

    // [REVERT] 优先按时间匹配
    const QString ts = faultsModel_->item(row, FCOL_TIME)->text();
    int logRow = findNearestLogRowByTime(ts);
    if (logRow >= 0) {
        logsView_->selectRow(logRow);
        logsView_->scrollTo(logsModel_->index(logRow, 0));
        return;
    }

    // [REVERT] 如果按时间找不到，再尝试按序号匹配
    const int fseq = faultsModel_->item(row, FCOL_SEQ)->text().toInt();
    logRow = findLogRowBySeq(fseq);
    if (logRow >= 0) {
        logsView_->selectRow(logRow);
        logsView_->scrollTo(logsModel_->index(logRow, 0));
        return;
    }

    // [REVERT] 如果两种方式都找不到，则提示用户
    QMessageBox::information(this, tr("提示"), tr("当前日志页未包含该故障对应的日志，请翻页后再试"));
}

void TicketsPageWidget::onSelectionChanged() {
    const auto idx = table_->currentIndex();
    bool enableJoin = false;

    if (idx.isValid()) {
        // [MOD] 从存储的完整数据中更新详情面板和“再次加入”按钮
        QVariant data = model_->item(idx.row(), COL_ID)->data(Qt::UserRole);
        if (data.isValid()) {
            QJsonObject fullData = data.toJsonObject();
            const int st = fullData.value("status").toInt();
            enableJoin = (st != 2 && st != 3); // 非已解决/关闭

            detailStatusLabel_->setText(QString::number(st));
            detailTimeLabel_->setText(fullData.value("created_time").toString());
            detailPriorityLabel_->setText(fullData.value("priority").toString());
            detailCreatorLabel_->setText(fullData.value("creator").toString());
            detailLogsCountLabel_->setText(QString::number(fullData.value("logs").toInt()));
            detailFaultsCountLabel_->setText(QString::number(fullData.value("faults").toInt()));
        }
    } else {
        // [ADD] 如果没有选中项，则清空所有详情标签
        detailStatusLabel_->setText("-");
        detailTimeLabel_->setText("-");
        detailPriorityLabel_->setText("-");
        detailCreatorLabel_->setText("-");
        detailLogsCountLabel_->setText("-");
        detailFaultsCountLabel_->setText("-");
    }
    btnJoin_->setEnabled(enableJoin);
}

void TicketsPageWidget::exportModelToCsv(QStandardItemModel* m, const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存到 %1").arg(path));
        return;
    }
    QTextStream ts(&f);
    QStringList header;
    for (int c=0; c<m->columnCount(); ++c) {
        if (m->headerData(c, Qt::Horizontal).toString() == tr("ID")) continue;
        header << m->headerData(c, Qt::Horizontal).toString();
    }
    ts << header.join(',') << "\n";
    for (int r=0; r<m->rowCount(); ++r) {
        QStringList cols;
        for (int c=0; c<m->columnCount(); ++c) {
            if (m->headerData(c, Qt::Horizontal).toString() == tr("ID")) continue;
            cols << m->item(r, c)->text();
        }
        ts << cols.join(',') << "\n";
    }
    f.close();
}

qint64 TicketsPageWidget::parseIso(const QString& s) const {
    QDateTime dt = QDateTime::fromString(s, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) dt = QDateTime::fromString(s, Qt::ISODate);
    if (!dt.isValid()) return -1;
    dt.setTimeSpec(Qt::LocalTime);
    return dt.toMSecsSinceEpoch();
}

int TicketsPageWidget::findLogRowBySeq(int seq) const {
    for (int r=0; r<logsModel_->rowCount(); ++r) {
        if (logsModel_->item(r, LCOL_SEQ)->text().toInt() == seq) return r;
    }
    return -1;
}

int TicketsPageWidget::findNearestLogRowByTime(const QString& isoTs) const {
    const qint64 target = parseIso(isoTs);
    if (target < 0) return -1;
    int bestRow = -1; qint64 bestDelta = LLONG_MAX;
    for (int r=0; r<logsModel_->rowCount(); ++r) {
        const QString t = logsModel_->item(r, LCOL_TIME)->text();
        const qint64 ms = const_cast<TicketsPageWidget*>(this)->parseIso(t);
        if (ms < 0) continue;
        const qint64 d = qAbs(ms - target);
        if (d < bestDelta) { bestDelta = d; bestRow = r; }
    }
    return bestRow;
}
