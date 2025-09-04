#include "workorderdetaildialog.h"
#include "ui_workorderdetaildialog.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QBrush>
#include <QColor>

WorkOrderDetailDialog::WorkOrderDetailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WorkOrderDetailDialog)
{
    ui->setupUi(this);
    
    // 初始化操作记录模型
    m_recordModel = new QStandardItemModel(this);
    m_recordModel->setHorizontalHeaderLabels(QStringList() << "时间" << "操作人员" << "操作内容");
    ui->recordTableView->setModel(m_recordModel);
    ui->recordTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 连接关闭按钮信号
    connect(ui->closeButton, &QPushButton::clicked, this, &WorkOrderDetailDialog::on_closeButton_clicked);
}

WorkOrderDetailDialog::~WorkOrderDetailDialog()
{
    delete ui;
}

void WorkOrderDetailDialog::setWorkOrderInfo(const QString& ticketId, const QString& deviceName, const QString& description,
                                            const QString& priority, const QString& submitTime, const QString& status)
{
    // 设置工单基本信息
    ui->ticketIdLabel->setText(ticketId);
    ui->deviceNameLabel->setText(deviceName);
    ui->descriptionLabel->setText(description);
    ui->priorityLabel->setText(priority);
    ui->submitTimeLabel->setText(submitTime);
    ui->statusLabel->setText(status);
    
    // 根据优先级设置不同颜色
    if (priority == "高") {
        ui->priorityLabel->setStyleSheet("color: red;");
    } else if (priority == "中") {
        ui->priorityLabel->setStyleSheet("color: orange;");
    } else if (priority == "低") {
        ui->priorityLabel->setStyleSheet("color: blue;");
    }
    
    // 根据状态设置不同颜色
    if (status == "待处理") {
        ui->statusLabel->setStyleSheet("color: orange;");
    } else if (status == "处理中") {
        ui->statusLabel->setStyleSheet("color: yellow;");
    } else if (status == "已完成") {
        ui->statusLabel->setStyleSheet("color: green;");
    }
}

void WorkOrderDetailDialog::setOperationRecords(const QJsonArray& records)
{
    m_recordModel->removeRows(0, m_recordModel->rowCount());
    
    for (const QJsonValue& value : records) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            QList<QStandardItem*> rowItems;
            
            // 时间
            QString timeText = obj.value("time").toString();
            if (timeText.contains('T')) {
                timeText = timeText.replace('T', ' ');
                if (timeText.length() > 16) {
                    timeText = timeText.left(16);
                }
            }
            rowItems << new QStandardItem(timeText);
            
            // 操作人员
            rowItems << new QStandardItem(obj.value("operator").toString());
            
            // 操作内容
            QString operationText = obj.value("operation").toString();
            QString operationType = obj.value("type").toString();
            
            QStandardItem* operationItem = new QStandardItem(operationText);
            
            // 根据操作类型设置不同颜色和图标
            if (operationType == "create") {
                operationItem->setForeground(QBrush(QColor(0, 0, 255))); // 蓝色
            } else if (operationType == "process") {
                operationItem->setForeground(QBrush(QColor(255, 165, 0))); // 橙色
            } else if (operationType == "complete") {
                operationItem->setForeground(QBrush(QColor(0, 128, 0))); // 绿色
            }
            
            rowItems << operationItem;
            m_recordModel->appendRow(rowItems);
        }
    }
}

void WorkOrderDetailDialog::on_closeButton_clicked()
{
    this->close();
}