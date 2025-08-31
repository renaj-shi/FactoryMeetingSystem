#include "devicedialog.h"
#include "ui_devicedialog.h"
#include <QtCharts>
#include <QRandomGenerator>
#include <QDateTime>
#include <cmath>
#include <QVector>
#include <QValueAxis>
#include <QVBoxLayout>
// 【增】为支持发送JSON数据，包含QJsonArray头文件
#include <QJsonArray>
// 【增】为支持文件对话框，包含相关头文件
#include <QFileDialog>
#include <QDir>

using namespace QtCharts;

DeviceDialog::DeviceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceDialogBase)
    , updateTimer(new QTimer(this))
    , model1(new QSqlQueryModel(this))
    , model2(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    setWindowTitle("设备参数监控");

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &DeviceDialog::onUpdateTimer);
    updateTimer->start(2000); // 每2秒更新一次
    setDeviceParams();

    // 【增】手动连接UI按钮的信号和槽，确保功能在所有环境下都有效
    // 注意: 请确保您的.ui文件中，“查看变化趋势”按钮的objectName是pushButton，“刷新历史数据”按钮的objectName是pushButton_2
    connect(ui->pushButton, &QPushButton::clicked, this, &DeviceDialog::on_pushButton_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DeviceDialog::on_pushButton_2_clicked);
}

DeviceDialog::~DeviceDialog()
{
    delete ui;
    // 【改】QTimer是this的子对象，会被自动析构，无需手动delete
    // delete updateTimer;
}

void DeviceDialog::setDeviceParams()
{
    QSqlQuery q;
    q.prepare("SELECT temperature, pressure, vibration, current, voltage, speed, status, record_time "
              "FROM device_history "
              "ORDER BY record_time DESC LIMIT 1");
    if (!q.exec() || !q.next()) {
        qDebug() << "读取 device_history 失败或无数据:" << q.lastError().text();
        return;
    }

    DeviceParams params;
    params.temperature = q.value("temperature").toDouble();
    params.pressure    = q.value("pressure").toDouble();
    params.vibration   = q.value("vibration").toDouble();
    params.current     = q.value("current").toDouble();
    params.voltage     = q.value("voltage").toDouble();
    params.speed       = q.value("speed").toInt();
    params.status      = q.value("status").toBool();
    params.lastUpdate  = q.value("record_time").toDateTime();

    deviceParams = params;

    // 更新界面
    ui->tempLabel->setText(QString::number(params.temperature, 'f', 1) + " °C");
    ui->pressureLabel->setText(QString::number(params.pressure, 'f', 1) + " kPa");
    ui->vibrationLabel->setText(QString::number(params.vibration, 'f', 1) + " mm/s");
    ui->currentLabel->setText(QString::number(params.current, 'f', 1) + " A");
    ui->voltageLabel->setText(QString::number(params.voltage, 'f', 1) + " V");
    ui->speedLabel->setText(QString::number(params.speed) + " RPM");
    ui->statusLabel->setText(params.status ? "运行中" : "停止");
    ui->updateTimeLabel->setText(params.lastUpdate.toString("yyyy-MM-dd hh:mm:ss"));

    // 阈值颜色
    ui->tempLabel->setStyleSheet(params.temperature > 70.0 ? "color: red;" : "color: black;");
    ui->pressureLabel->setStyleSheet(params.pressure > 130.0 ? "color: red;" : "color: black;");
    ui->vibrationLabel->setStyleSheet(params.vibration > 4.0 ? "color: red;" : "color: black;");

    // 【增】根据新设计，在每次数据更新时，组装JSON并通过信号发射出去，作为数据源
    QJsonObject obj;
    obj["ts"]         = deviceParams.lastUpdate.toMSecsSinceEpoch();
    obj["temperature"]= deviceParams.temperature;
    obj["pressure"]   = deviceParams.pressure;
    obj["vibration"]  = deviceParams.vibration;
    obj["current"]    = deviceParams.current;
    obj["voltage"]    = deviceParams.voltage;
    obj["speed"]      = deviceParams.speed;
    obj["status"]     = deviceParams.status ? 1 : 0;

    // 可选：生成日志/故障
    QJsonArray logs;
    logs.append(QString("T=%1,P=%2,V=%3").arg(deviceParams.temperature,0,'f',1)
                    .arg(deviceParams.pressure,0,'f',1)
                    .arg(deviceParams.vibration,0,'f',2));
    obj["logs"] = logs;

    QJsonArray faults;
    if (deviceParams.temperature > 70.0) {
        QJsonObject f; f["code"]="F_TEMP_HIGH"; f["text"]="温度过高"; f["level"]="中";
        faults.append(f);
    }
    if (deviceParams.pressure > 130.0) {
        QJsonObject f; f["code"]="F_PRESS_HIGH"; f["text"]="压力过高"; f["level"]="中";
        faults.append(f);
    }
    if (deviceParams.vibration > 4.0) {
        QJsonObject f; f["code"]="F_VIB_HIGH"; f["text"]="振动过大"; f["level"]="中";
        faults.append(f);
    }
    obj["faults"] = faults;

    emit paramsUpdated(obj);
}

// 定时更新设备数据（在类的头文件中声明为槽函数）
void DeviceDialog::onUpdateTimer()
{
    setDeviceParams();
}

void DeviceDialog::on_close_clicked()
{
    close();
}

void DeviceDialog::on_deleteHistoryButton_clicked()
{
    QSqlQuery q;
    if (!q.exec("DELETE FROM device_history")) {
        QMessageBox::warning(this, "失败",
                             "清空失败：" + q.lastError().text());
        return;
    }

    // 让自增主键从 1 重新开始（可选）
    q.exec("DELETE FROM sqlite_sequence WHERE name='device_history'");

    QMessageBox::information(this, "完成", "全部设备历史数据已清空！");
    refreshHistoryTable();   // 重新加载空表
}

void DeviceDialog::refreshHistoryTable()
{
    // 使用setQuery设置模型数据
    model1->setQuery("SELECT id,temperature,pressure,vibration,current,voltage,speed,status,record_time  "
                     "FROM device_history ORDER BY record_time DESC");

    if (model1->lastError().isValid()) {
        QMessageBox::warning(this, "模型错误", "设置模型数据失败: " + model1->lastError().text());
        return;
    }

    // 设置中文表头
    model1->setHeaderData(0, Qt::Horizontal, "编号");
    model1->setHeaderData(1, Qt::Horizontal, "温度(°C)");
    model1->setHeaderData(2, Qt::Horizontal, "压力(kPa)");
    model1->setHeaderData(3, Qt::Horizontal, "振动(mm/s)");
    model1->setHeaderData(4, Qt::Horizontal, "电流(A)");
    model1->setHeaderData(5, Qt::Horizontal, "电压(V)");
    model1->setHeaderData(6, Qt::Horizontal, "转速(RPM)");
    model1->setHeaderData(7, Qt::Horizontal, "状态");
    model1->setHeaderData(8, Qt::Horizontal, "记录时间");

    ui->tableHistoryView->setModel(model1);
    ui->tableHistoryView->resizeColumnsToContents();

    // 【改】使用更合适的标签来显示状态信息
    // 显示状态信息
    // ui->statusLabel->setText(QString("已加载 %1 条记录").arg(model1->rowCount()));
}

void DeviceDialog::on_pushButton_2_clicked()
{
    refreshHistoryTable();
}

void DeviceDialog::on_pushButton_clicked()
{
    QString column = ui->comboBox->currentText();

    // 建立显示名称到数据库列名的映射
    QMap<QString, QString> columnMap;
    columnMap["编号"] = "id";
    columnMap["温度"] = "temperature";
    columnMap["压力"] = "pressure";
    columnMap["振动"] = "vibration";
    columnMap["电流"] = "current";
    columnMap["电压"] = "voltage";
    columnMap["转速"] = "speed";
    columnMap["状态"] = "status";
    columnMap["记录时间"] = "record_time";

    QString dbColumn = columnMap.value(column);
    if (dbColumn.isEmpty()) {
        QMessageBox::warning(this, "错误", "无效的列名");
        return;
    }

    // SQL查询
    QString queryStr = QString("SELECT %1 FROM device_history ORDER BY record_time DESC").arg(dbColumn);
    model2->setQuery(queryStr);

    if (model2->lastError().isValid()) {
        QMessageBox::warning(this, "查询错误", model2->lastError().text());
        return;
    }

    // 将获取到的model转化为数组
    QVector<double> data;
    // 【改】删除了对 model2 的非空检查，因为它不可能为空

    int rowCount = model2->rowCount();
    if (rowCount == 0) {
        QMessageBox::information(this, "提示", "没有数据可显示");
        return;
    }

    data.reserve(rowCount);

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex index = model2->index(i, 0); // 只有一列，所以列索引为0
        QVariant value = model2->data(index);
        bool ok;
        double numericValue = value.toDouble(&ok);
        if (ok) { // 【改】简化了检查逻辑
            data.append(numericValue);
        } else {
            data.append(0.0);
            qDebug() << "行" << i << "数据转换失败:" << value.toString();
        }
    }

    // 创建折线图系列
    QLineSeries *series = new QLineSeries();
    series->setName(column);

    // 添加数据点
    for (int i = 0; i < data.size(); ++i) {
        series->append(i, data[i]); // x轴为索引，y轴为数据值
    }

    // 创建图表
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(column + " 趋势图");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // 【改】使用 Qt 6 兼容的 createDefaultAxes() 来简化
    chart->createDefaultAxes();
    //【删】删除了手动创建坐标轴和设置范围的复杂代码

    // 创建图表视图
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 创建对话框显示图表
    QDialog *chartDialog = new QDialog(this);
    chartDialog->setWindowTitle(column + " 趋势图");
    chartDialog->setModal(false);

    QVBoxLayout *layout = new QVBoxLayout(chartDialog);
    layout->addWidget(chartView);

    // 添加一些控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("关闭", chartDialog);
    QPushButton *saveButton = new QPushButton("保存图片", chartDialog);

    connect(closeButton, &QPushButton::clicked, chartDialog, &QDialog::accept);
    connect(saveButton, &QPushButton::clicked, this, [chartView, column]() {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "保存图表",
                                                        QDir::homePath() + "/" + column + "_chart.png",
                                                        "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg)");

        if (!fileName.isEmpty()) {
            QPixmap pixmap = chartView->grab();
            pixmap.save(fileName);
        }
    });

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    // 设置对话框大小
    chartDialog->resize(900, 600);
    // 显示对话框
    chartDialog->show();
    // 可选：对话框关闭时自动删除
    chartDialog->setAttribute(Qt::WA_DeleteOnClose);
}

void DeviceDialog::on_cautionButton_clicked()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "创建工单",
                                  "检测到设备异常，是否立即创建维修工单？",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    // 【改】简化了用户信息获取，直接使用默认值
    QString currentUser = "服务端监控人员";
    QString description = "设备参数异常，需要检修";

    /* ---------- 组装 device_params JSON ---------- */
    QJsonObject obj;
    obj["temperature"]  = deviceParams.temperature;
    obj["pressure"]     = deviceParams.pressure;
    obj["vibration"]    = deviceParams.vibration;
    obj["current"]      = deviceParams.current;
    obj["voltage"]      = deviceParams.voltage;
    obj["speed"]        = deviceParams.speed;
    QString deviceParamsJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    /* ---------- 计算优先级 ---------- */
    QString priority = "中";
    if (deviceParams.temperature > 80.0 || deviceParams.pressure > 140.0 || deviceParams.vibration > 5.0)
        priority = "高";
    else if (deviceParams.temperature > 70.0 || deviceParams.pressure > 130.0 || deviceParams.vibration > 4.0)
        priority = "中";
    else
        priority = "低";

    /* ---------- 数据库操作 ---------- */
    QSqlQuery query;
    query.prepare("INSERT INTO tickets "
                  "(title, description, priority, status, creator, expert_username, "
                  "created_time, updated_time, device_params) "
                  "VALUES (:title, :desc, :priority, :status, :creator, :expert, "
                  ":created, :updated, :device_params)");

    query.bindValue(":title",  "设备异常维修工单");
    query.bindValue(":desc",   description);
    query.bindValue(":priority", priority);
    query.bindValue(":status", 0);
    query.bindValue(":creator", currentUser);
    query.bindValue(":expert",  QVariant(QVariant::String));
    query.bindValue(":created", QDateTime::currentDateTime());
    query.bindValue(":updated", QDateTime::currentDateTime());
    query.bindValue(":device_params", deviceParamsJson);

    if (query.exec()) {
        int ticketId = query.lastInsertId().toInt();
        QMessageBox::information(this, "成功",
                                 QString("工单创建成功！\n工单编号: %1\n优先级: %2")
                                     .arg(ticketId).arg(priority));
        // 【改】将原来的注释代码改为有效的信号发射
        emit workOrderCreated(ticketId, "设备异常维修工单");
    } else {
        QMessageBox::critical(this, "错误",
                              QString("工单创建失败: %1").arg(query.lastError().text()));
    }
}
