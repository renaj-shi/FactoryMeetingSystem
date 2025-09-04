#include "ticketsdialog.h"
#include "ui_ticketsdialog.h"
#include "mainwindow.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>  // ← 添加这一行
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

TicketsDialog::TicketsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TicketsDialog)
    , model(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    setWindowTitle("工单列表");
    setupUI();
    // 绑定刷新按钮
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &TicketsDialog::refreshTable);

    refreshTable();   // 首次打开时加载
}

TicketsDialog::~TicketsDialog()
{
    delete ui;
}

void TicketsDialog::refreshTable()
{
    model->setQuery("SELECT id, title, description, priority, status, creator, expert_username, created_time "
                    "FROM tickets ORDER BY created_time DESC");
    if (model->lastError().isValid())
        qDebug() << "TicketsDialog query error:" << model->lastError().text();

    // 设置中文表头
    model->setHeaderData(0, Qt::Horizontal, "编号");
    model->setHeaderData(1, Qt::Horizontal, "标题");
    model->setHeaderData(2, Qt::Horizontal, "描述");
    model->setHeaderData(3, Qt::Horizontal, "优先级");
    model->setHeaderData(4, Qt::Horizontal, "状态");
    model->setHeaderData(5, Qt::Horizontal, "创建者");
    model->setHeaderData(6, Qt::Horizontal, "指派专家");
    model->setHeaderData(7, Qt::Horizontal, "创建时间");

    ui->tableViewTickets->setModel(model);
    ui->tableViewTickets->resizeColumnsToContents();   // 自动列宽
}

void TicketsDialog::changeTickets()
{
    /* 1. 输入工单 id */
    bool ok = false;
    int ticketId = QInputDialog::getInt(this, "修改工单",
                                        "请输入要修改的工单编号：", 1, 1, INT_MAX, 1, &ok);
    if (!ok) return;

    /* 2. 选择列名（允许手动输入，但先给常用列提示） */
    QStringList columns = { "title", "description", "priority",
                           "status", "solution", "expert_username", "device_params" };
    QString column = QInputDialog::getItem(this, "选择列",
                                           "请选择要修改的列：",
                                           columns, 0, false, &ok);
    if (!ok || column.isEmpty()) return;

    /* 3. 输入新内容 */
    QString newValue = QInputDialog::getText(this, "修改内容",
                                             QString("请输入 %1 的新值：").arg(column),
                                             QLineEdit::Normal, "", &ok);
    if (!ok || newValue.isEmpty()) return;

    /* 4. 构造并执行更新语句 */
    QSqlQuery q;
    q.prepare(QString("UPDATE tickets "
                      "SET %1 = :value, updated_time = CURRENT_TIMESTAMP "
                      "WHERE id = :id").arg(column));
    q.bindValue(":value", newValue);
    q.bindValue(":id",    ticketId);

    if (q.exec() && q.numRowsAffected() > 0) {
        QMessageBox::information(this, "成功",
                                 QString("工单 %1 的 %2 已更新为 \"%3\"")
                                     .arg(ticketId).arg(column).arg(newValue));
    } else {
        QMessageBox::warning(this, "失败",
                             QString("更新失败：\n%1").arg(q.lastError().text()));
    }
}

void TicketsDialog::on_refreshButton_clicked()
{
    refreshTable();
}

void TicketsDialog::on_deleteButton_clicked()
{
    QSqlQuery q;
    if (!q.exec("DELETE FROM tickets")) {
        QMessageBox::warning(this, "失败",
                             "清空失败：" + q.lastError().text());
        return;
    }

    // 让自增主键从 1 重新开始（可选）
    q.exec("DELETE FROM sqlite_sequence WHERE name='tickets'");

    QMessageBox::information(this, "完成", "全部工单已清空！");
    refreshTable();   // 重新加载空表
}

void TicketsDialog::on_changeButton_clicked()
{
    changeTickets();
}

void TicketsDialog::setupUI()
{
    // 设置对话框基本属性
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setFixedSize(2000, 1400); // 固定大小适应2/3区域

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 创建标题标签
    QLabel *titleLabel = new QLabel("工单管理系统", this);
    titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #2c3e50; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 创建按钮工具栏
    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // 刷新按钮
    ui->refreshButton = new QPushButton("刷新", buttonWidget);
    ui->refreshButton->setIcon(QIcon(":/icons/refresh.svg"));
    ui->refreshButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; background-color: #3498db; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #2980b9; }"
        );

    // 修改按钮
    ui->changeButton = new QPushButton("修改工单", buttonWidget);
    ui->changeButton->setIcon(QIcon(":/icons/edit.svg"));
    ui->changeButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; background-color: #f39c12; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #e67e22; }"
        );

    // 删除按钮
    ui->deleteButton = new QPushButton("清空工单", buttonWidget);
    ui->deleteButton->setIcon(QIcon(":/icons/delete.svg"));
    ui->deleteButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; background-color: #e74c3c; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #c0392b; }"
        );

    // 关闭按钮
    ui->pushButton = new QPushButton("关闭", buttonWidget);
    ui->pushButton->setIcon(QIcon(":/icons/close.svg"));
    ui->pushButton->setStyleSheet(
        "QPushButton { padding: 8px 16px; background-color: #7f8c8d; color: white; border-radius: 4px; }"
        "QPushButton:hover { background-color: #636e72; }"
        );

    // 添加按钮到布局
    buttonLayout->addWidget(ui->refreshButton);
    buttonLayout->addWidget(ui->changeButton);
    buttonLayout->addWidget(ui->deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(ui->pushButton);

    mainLayout->addWidget(buttonWidget);

    // 创建表格视图
    ui->tableViewTickets = new QTableView(this);
    ui->tableViewTickets->setStyleSheet(
        "QTableView {"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 4px;"
        "   background-color: #ffffff;"
        "   gridline-color: #ecf0f1;"
        "}"
        "QTableView::item:selected {"
        "   background-color: #3498db;"
        "   color: white;"
        "}"
        "QHeaderView::section {"
        "   background-color: #34495e;"
        "   color: white;"
        "   padding: 6px;"
        "   border: none;"
        "}"
        );

    // 设置表格属性
    ui->tableViewTickets->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewTickets->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewTickets->setAlternatingRowColors(true);
    ui->tableViewTickets->setSortingEnabled(true);
    ui->tableViewTickets->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewTickets->verticalHeader()->setVisible(false);

    mainLayout->addWidget(ui->tableViewTickets, 1); // 表格占据剩余空间

    // 状态栏
    QLabel *statusLabel = new QLabel("双击工单可查看详细信息", this);
    statusLabel->setStyleSheet("QLabel { color: #7f8c8d; font-size: 12px; }");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // 连接信号槽
    connect(ui->refreshButton, &QPushButton::clicked, this, &TicketsDialog::refreshTable);
    connect(ui->changeButton, &QPushButton::clicked, this, &TicketsDialog::changeTickets);
    connect(ui->deleteButton, &QPushButton::clicked, this, &TicketsDialog::on_deleteButton_clicked);
    connect(ui->pushButton, &QPushButton::clicked, this, &TicketsDialog::close);
}
