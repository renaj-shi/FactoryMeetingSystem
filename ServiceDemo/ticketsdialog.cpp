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

void TicketsDialog::on_pushButton_clicked()
{
    close();
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

