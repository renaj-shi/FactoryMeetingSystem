#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include <QRandomGenerator>
#include <QSqlQueryModel>
// 【增】为支持发送JSON数据，包含QJsonObject头文件
#include <QJsonObject>

namespace Ui {
class DeviceDialogBase;
}

class DeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceDialog(QWidget *parent = nullptr);
    ~DeviceDialog();

    void setDeviceParams();

    // 【增】根据新设计，增加信号，用于将数据和事件发送给MainWindow
signals:
    void paramsUpdated(const QJsonObject& json);
    void workOrderCreated(int ticketId, const QString& title);

private slots:
    void on_close_clicked();
    void onUpdateTimer();
    void on_deleteHistoryButton_clicked();
    void refreshHistoryTable();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();

    void on_cautionButton_clicked();

private:
    Ui::DeviceDialogBase *ui;
    DeviceParams deviceParams;
    QTimer *updateTimer;
    QSqlQueryModel *model1=nullptr;
    QSqlQueryModel *model2=nullptr;
};

#endif // DEVICEDIALOG_H
