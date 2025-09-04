#ifndef WORKORDERDETAILDIALOG_H
#define WORKORDERDETAILDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class WorkOrderDetailDialog;
}

class WorkOrderDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WorkOrderDetailDialog(QWidget *parent = nullptr);
    ~WorkOrderDetailDialog();
    
    // 设置工单详情信息
    void setWorkOrderInfo(const QString& ticketId, const QString& deviceName, const QString& description,
                          const QString& priority, const QString& submitTime, const QString& status);
    
    // 设置操作记录
    void setOperationRecords(const QJsonArray& records);

private slots:
    void on_closeButton_clicked();

private:
    Ui::WorkOrderDetailDialog *ui;
    QStandardItemModel* m_recordModel; // 操作记录模型
};

#endif // WORKORDERDETAILDIALOG_H