#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include <QRandomGenerator>
#include <QSqlQueryModel>
#include <QLabel>
#include <QComboBox>
#include <QTableView>

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
    void setupUI();
    void setDeviceParams();

    // 【增】根据新设计，增加信号，用于将数据和事件发送给MainWindow
signals:
    void paramsUpdated(const QJsonObject& json);
    void workOrderCreated(int ticketId, const QString& title);

private slots:
    void on_closeButton_clicked();
    void onUpdateTimer();
    void on_deleteHistoryButton_clicked();
    void refreshHistoryTable();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();

    void on_cautionButton_clicked();

private:
    QLabel* getValueLabel(int index);

    QLabel *tempLabel;
    QLabel *pressureLabel;
    QLabel *vibrationLabel;
    QLabel *currentLabel;
    QLabel *voltageLabel;
    QLabel *speedLabel;
    QLabel *statusLabel;
    QLabel *updateTimeLabel;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *deleteHistoryButton;
    QPushButton *cautionButton;
    QPushButton *closeButton;
    QComboBox *comboBox;
    QTableView *tableHistoryView;
    Ui::DeviceDialogBase *ui;
    DeviceParams deviceParams;
    QTimer *updateTimer;
    QSqlQueryModel *model1=nullptr;
    QSqlQueryModel *model2=nullptr;
    QSqlDatabase m_db;
};

#endif // DEVICEDIALOG_H
