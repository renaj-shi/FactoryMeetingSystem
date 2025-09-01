#include "workorderdialog.h"
#include "ui_workorderdialog.h"
#include <QDebug>

WorkOrderDialog::WorkOrderDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkOrderDialog),
    m_priorityGroup(new QButtonGroup(this))
{
    ui->setupUi(this);
    m_charCountLabel = ui->charCountLabel;

    // 将单选按钮添加到按钮组
    m_priorityGroup->addButton(ui->highPriorityBtn);
    m_priorityGroup->addButton(ui->mediumPriorityBtn);
    m_priorityGroup->addButton(ui->lowPriorityBtn);

    // 连接文本变化信号以更新字数统计
    connect(ui->descEdit, &QTextEdit::textChanged, this, &WorkOrderDialog::onTextChanged);

    // 连接创建工单按钮的点击信号
    connect(ui->createButton, &QPushButton::clicked, this, &WorkOrderDialog::onCreateTicketClicked);
}

WorkOrderDialog::~WorkOrderDialog()
{
    delete ui;
}

QString WorkOrderDialog::getTitle() const
{
    return ui->titleEdit->text();
}

QString WorkOrderDialog::getDescription() const
{
    return ui->descEdit->toPlainText();
}

QString WorkOrderDialog::getPriority() const
{
    if (ui->highPriorityBtn->isChecked()) {
        return "high";
    } else if (ui->mediumPriorityBtn->isChecked()) {
        return "medium";
    } else if (ui->lowPriorityBtn->isChecked()) {
        return "low";
    }
    return "high"; // 默认返回高优先级
}

void WorkOrderDialog::setResultText(const QString &text)
{
    ui->resultLabel->setText(text);
}

void WorkOrderDialog::onTextChanged()
{
    int count = ui->descEdit->toPlainText().length();
    m_charCountLabel->setText(QString("%1/500").arg(count));
    
    // 如果超过500字，截断文本
    if (count > 500) {
        QString text = ui->descEdit->toPlainText();
        ui->descEdit->blockSignals(true); // 防止递归
        ui->descEdit->setPlainText(text.left(500));
        ui->descEdit->moveCursor(QTextCursor::End);
        ui->descEdit->blockSignals(false);
    }
}

void WorkOrderDialog::onCreateTicketClicked()
{
    // 发出创建工单信号
    emit createTicket();
}