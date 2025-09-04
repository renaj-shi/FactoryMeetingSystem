#include "ticketsviewdialog.h"

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

#include "ticketsclient.h"
#include "workordermanager.h"

// 工单列表列索引
static constexpr int COL_ID       = 0;
static constexpr int COL_TITLE    = 1;
static constexpr int COL_PRIORITY = 2;
static constexpr int COL_STATUS   = 3;
static constexpr int COL_CREATOR  = 4;
static constexpr int COL_CTIME    = 5;
static constexpr int COL_LOGS     = 6;
static constexpr int COL_FAULTS   = 7;

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

TicketsViewDialog::TicketsViewDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("历史工单（含日志/故障）"));
    resize(1180, 680);

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
    model_->setHorizontalHeaderLabels(QStringList()
                                      << tr("编号") << tr("标题") << tr("优先级") << tr("状态")
                                      << tr("创建者") << tr("创建时间") << tr("日志数") << tr("故障数"));
    table_->setModel(model_);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);

    // 右：日志 + 故障
    auto* right = new QWidget(split);
    auto* rightLy = new QVBoxLayout(right);

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
    logsView_->setColumnHidden(LCOL_ID, true); // 隐藏数据库 id

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
    faultsView_->setColumnHidden(FCOL_ID, true); // 隐藏数据库 id

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
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 6);
    root->addWidget(split, 1);

    // 事件绑定
    connect(btnRefresh_, &QPushButton::clicked, this, &TicketsViewDialog::refresh);
    connect(btnJoin_,    &QPushButton::clicked, this, &TicketsViewDialog::onJoinSelected);

    connect(btnLogs_,        &QPushButton::clicked, this, &TicketsViewDialog::onLoadLogs);
    connect(btnExportLogs_,  &QPushButton::clicked, this, &TicketsViewDialog::onExportLogs);
    connect(logsPrev_,       &QPushButton::clicked, this, [this]{ logsPage_->setValue(qMax(1, logsPage_->value()-1)); onLoadLogs(); });
    connect(logsNext_,       &QPushButton::clicked, this, [this]{ logsPage_->setValue(logsPage_->value()+1); onLoadLogs(); });

    connect(btnFaults_,        &QPushButton::clicked, this, &TicketsViewDialog::onLoadFaults);
    connect(btnExportFaults_,  &QPushButton::clicked, this, &TicketsViewDialog::onExportFaults);
    connect(btnLocate_,        &QPushButton::clicked, this, &TicketsViewDialog::onLocateFromFault);
    connect(faultsPrev_,       &QPushButton::clicked, this, [this]{ faultsPage_->setValue(qMax(1, faultsPage_->value()-1)); onLoadFaults(); });
    connect(faultsNext_,       &QPushButton::clicked, this, [this]{ faultsPage_->setValue(faultsPage_->value()+1); onLoadFaults(); });

    connect(table_->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex&, const QModelIndex&){ onSelectionChanged(); });
}

void TicketsViewDialog::bindServices(TicketsClient* cli, WorkOrderManager* orders) {
    cli_ = cli; orders_ = orders;
    if (cli_) {
        connect(cli_, &TicketsClient::listed,       this, &TicketsViewDialog::onListed);
        connect(cli_, &TicketsClient::listFailed,   this, &TicketsViewDialog::onListFailed);
        connect(cli_, &TicketsClient::logsListed,   this, &TicketsViewDialog::onLogsListed);
        connect(cli_, &TicketsClient::logsFailed,   this, &TicketsViewDialog::onLogsFailed);
        connect(cli_, &TicketsClient::faultsListed, this, &TicketsViewDialog::onFaultsListed);
        connect(cli_, &TicketsClient::faultsFailed, this, &TicketsViewDialog::onFaultsFailed);
    }
}

void TicketsViewDialog::refresh() {
    if (cli_) cli_->list(1, 200);
}

int TicketsViewDialog::selectedId() const {
    const auto idx = table_->currentIndex();
    if (!idx.isValid()) return -1;
    return model_->item(idx.row(), COL_ID)->text().toInt();
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
        row << new QStandardItem(QString::number(o.value("logs").toInt()));
        row << new QStandardItem(QString::number(o.value("faults").toInt()));
        model_->appendRow(row);
    }
    onSelectionChanged();
}

void TicketsViewDialog::onListFailed(const QString& reason) {
    QMessageBox::warning(this, tr("加载失败"), reason);
}

void TicketsViewDialog::onLoadLogs() {
    if (!cli_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选择工单")); return; }
    currentTicketId_ = id;
    cli_->logs(id, logsPage_->value(), logsPageSize_->value());
}

void TicketsViewDialog::onLoadFaults() {
    if (!cli_) return;
    const int id = selectedId();
    if (id <= 0) { QMessageBox::information(this, tr("提示"), tr("请先选择工单")); return; }
    currentTicketId_ = id;
    cli_->faults(id, faultsPage_->value(), faultsPageSize_->value());
}

void TicketsViewDialog::onLogsListed(int ticketId, const QJsonArray& rows) {
    Q_UNUSED(ticketId);
    logsModel_->removeRows(0, logsModel_->rowCount());
    for (const auto& v : rows) {
        const auto o = v.toObject();
        const int seq = o.value("seq").toInt();         // 每工单序号
        const int id  = o.value("id").toInt();          // 数据库 id（隐藏列）

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
        r << new QStandardItem(QString::number(id)); // 隐藏列
        logsModel_->appendRow(r);
    }
    logsView_->setColumnHidden(LCOL_ID, true); // 保证隐藏
}

void TicketsViewDialog::onLogsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("日志加载失败"), reason);
}

void TicketsViewDialog::onFaultsListed(int ticketId, const QJsonArray& rows) {
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
        r << new QStandardItem(QString::number(id)); // 隐藏列
        faultsModel_->appendRow(r);
    }
    faultsView_->setColumnHidden(FCOL_ID, true);
}

void TicketsViewDialog::onFaultsFailed(const QString& reason) {
    QMessageBox::warning(this, tr("故障加载失败"), reason);
}

void TicketsViewDialog::onExportLogs() {
    const QString path = QFileDialog::getSaveFileName(this, tr("导出日志为 CSV"),
                                                      tr("logs_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    exportModelToCsv(logsModel_, path);
}

void TicketsViewDialog::onExportFaults() {
    const QString path = QFileDialog::getSaveFileName(this, tr("导出故障为 CSV"),
                                                      tr("faults_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    exportModelToCsv(faultsModel_, path);
}

void TicketsViewDialog::onLocateFromFault() {
    const auto idx = faultsView_->currentIndex();
    if (!idx.isValid()) { QMessageBox::information(this, tr("提示"), tr("请先选中一条故障")); return; }
    const int row = idx.row();
    const int fseq = faultsModel_->item(row, FCOL_SEQ)->text().toInt();

    // 优先按 seq 精确定位
    int logRow = findLogRowBySeq(fseq);
    if (logRow >= 0) {
        logsView_->selectRow(logRow);
        logsView_->scrollTo(logsModel_->index(logRow, 0));
        return;
    }
    // 若日志页里暂无该 seq，降级为按时间最近（要求故障 ts、日志 record_time 都在当前页）
    const QString ts = faultsModel_->item(row, FCOL_TIME)->text();
    logRow = findNearestLogRowByTime(ts);
    if (logRow >= 0) {
        logsView_->selectRow(logRow);
        logsView_->scrollTo(logsModel_->index(logRow, 0));
    } else {
        QMessageBox::information(this, tr("提示"), tr("当前日志页未包含该故障对应时间/序号，请翻页后再试"));
    }
}

void TicketsViewDialog::onSelectionChanged() {
    const auto idx = table_->currentIndex();
    bool enableJoin = false;
    if (idx.isValid()) {
        const int st = model_->item(idx.row(), COL_STATUS)->text().toInt();
        enableJoin = (st != 2 && st != 3); // 非已解决/关闭
    }
    btnJoin_->setEnabled(enableJoin);
}

void TicketsViewDialog::exportModelToCsv(QStandardItemModel* m, const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存到 %1").arg(path));
        return;
    }
    QTextStream ts(&f);
    QStringList header;
    for (int c=0; c<m->columnCount(); ++c) {
        if (m->headerData(c, Qt::Horizontal).toString() == tr("ID")) continue; // 跳过隐藏ID
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

qint64 TicketsViewDialog::parseIso(const QString& s) const {
    QDateTime dt = QDateTime::fromString(s, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) dt = QDateTime::fromString(s, Qt::ISODate);
    if (!dt.isValid()) return -1;
    dt.setTimeSpec(Qt::LocalTime);
    return dt.toMSecsSinceEpoch();
}

int TicketsViewDialog::findLogRowBySeq(int seq) const {
    for (int r=0; r<logsModel_->rowCount(); ++r) {
        if (logsModel_->item(r, LCOL_SEQ)->text().toInt() == seq) return r;
    }
    return -1;
}

int TicketsViewDialog::findNearestLogRowByTime(const QString& isoTs) const {
    const qint64 target = parseIso(isoTs);
    if (target < 0) return -1;
    int bestRow = -1; qint64 bestDelta = LLONG_MAX;
    for (int r=0; r<logsModel_->rowCount(); ++r) {
        const QString t = logsModel_->item(r, LCOL_TIME)->text();
        const qint64 ms = const_cast<TicketsViewDialog*>(this)->parseIso(t);
        if (ms < 0) continue;
        const qint64 d = qAbs(ms - target);
        if (d < bestDelta) { bestDelta = d; bestRow = r; }
    }
    return bestRow;
}
