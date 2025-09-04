#include "devicedialog.h"
#include "ui_devicedialog.h"
#include <QtCharts>
#include <QRandomGenerator>
#include <QDateTime>
#include <cmath>
#include <QVector>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

using namespace QtCharts;

DeviceDialog::DeviceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceDialogBase),
    updateTimer(nullptr),
    model1(new QSqlQueryModel(this)),
    model2(new QSqlQueryModel(this))
{
    ui->setupUi(this);
    setupUI();  // 改用自定义的setupUI
    setWindowTitle("设备参数监控");

    // 只创建一次计时器
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &DeviceDialog::onUpdateTimer);
    updateTimer->start(2000); // 每2秒更新一次
    setDeviceParams();

    // 手动连接按钮（确保各环境生效）
    connect(ui->pushButton,   &QPushButton::clicked, this, &DeviceDialog::on_pushButton_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DeviceDialog::on_pushButton_2_clicked);
}

DeviceDialog::~DeviceDialog()
{
    delete ui;
    // updateTimer 为子对象，无需手动 delete
}

void DeviceDialog::setDeviceParams()
{
    QSqlQuery q;
    q.prepare("SELECT temperature, pressure, vibration, current, voltage, speed, status, record_time "
              "FROM device_history ORDER BY record_time DESC LIMIT 1");
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

    // 组装JSON（供需要的上层使用）
    QJsonObject obj;
    obj["ts"]          = deviceParams.lastUpdate.toMSecsSinceEpoch();
    obj["temperature"] = deviceParams.temperature;
    obj["pressure"]    = deviceParams.pressure;
    obj["vibration"]   = deviceParams.vibration;
    obj["current"]     = deviceParams.current;
    obj["voltage"]     = deviceParams.voltage;
    obj["speed"]       = deviceParams.speed;
    obj["status"]      = deviceParams.status ? 1 : 0;

    QJsonArray logs;
    logs.append(QString("T=%1,P=%2,V=%3")
                    .arg(deviceParams.temperature,0,'f',1)
                    .arg(deviceParams.pressure,   0,'f',1)
                    .arg(deviceParams.vibration,  0,'f',2));
    obj["logs"] = logs;

    QJsonArray faults;
    if (deviceParams.temperature > 70.0) {
        QJsonObject f; f["code"]="F_TEMP_HIGH"; f["text"]="温度过高"; f["level"]="中"; faults.append(f);
    }
    if (deviceParams.pressure > 130.0) {
        QJsonObject f; f["code"]="F_PRESS_HIGH"; f["text"]="压力过高"; f["level"]="中"; faults.append(f);
    }
    if (deviceParams.vibration > 4.0) {
        QJsonObject f; f["code"]="F_VIB_HIGH"; f["text"]="振动过大"; f["level"]="中"; faults.append(f);
    }
    obj["faults"] = faults;

    emit paramsUpdated(obj);
}

void DeviceDialog::onUpdateTimer()
{
    setDeviceParams();
}

void DeviceDialog::on_closeButton_clicked()
{
    close();
}

void DeviceDialog::on_deleteHistoryButton_clicked()
{
    QSqlQuery q;
    if (!q.exec("DELETE FROM device_history")) {
        QMessageBox::warning(this, "失败", "清空失败：" + q.lastError().text());
        return;
    }
    q.exec("DELETE FROM sqlite_sequence WHERE name='device_history'");
    QMessageBox::information(this, "完成", "全部设备历史数据已清空！");
    refreshHistoryTable();
}

void DeviceDialog::refreshHistoryTable()
{
    model1->setQuery("SELECT id,temperature,pressure,vibration,current,voltage,speed,status,record_time "
                     "FROM device_history ORDER BY record_time DESC");

    if (model1->lastError().isValid()) {
        QMessageBox::warning(this, "模型错误", "设置模型数据失败: " + model1->lastError().text());
        return;
    }
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
}

void DeviceDialog::on_pushButton_2_clicked()
{
    refreshHistoryTable();
}

void DeviceDialog::on_pushButton_clicked()
{
    QString column = ui->comboBox->currentText();

    QMap<QString, QString> columnMap;
    columnMap["编号"]   = "id";
    columnMap["温度"]   = "temperature";
    columnMap["压力"]   = "pressure";
    columnMap["振动"]   = "vibration";
    columnMap["电流"]   = "current";
    columnMap["电压"]   = "voltage";
    columnMap["转速"]   = "speed";
    columnMap["状态"]   = "status";
    columnMap["记录时间"] = "record_time";

    const QString dbColumn = columnMap.value(column);
    if (dbColumn.isEmpty()) { QMessageBox::warning(this, "错误", "无效的列名"); return; }

    QString queryStr = QString("SELECT %1 FROM device_history ORDER BY record_time DESC").arg(dbColumn);
    model2->setQuery(queryStr);
    if (model2->lastError().isValid()) {
        QMessageBox::warning(this, "查询错误", model2->lastError().text());
        return;
    }

    QVector<double> data;
    const int rowCount = model2->rowCount();
    if (rowCount == 0) { QMessageBox::information(this, "提示", "没有数据可显示"); return; }
    data.reserve(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        const QModelIndex index = model2->index(i, 0);
        bool ok=false;
        const double v = model2->data(index).toDouble(&ok);
        data.append(ok ? v : 0.0);
    }

    QLineSeries *series = new QLineSeries();
    series->setName(column);
    for (int i = 0; i < data.size(); ++i) series->append(i, data[i]);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(column + " 趋势图");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->createDefaultAxes();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog *chartDialog = new QDialog(this);
    chartDialog->setWindowTitle(column + " 趋势图");
    chartDialog->setModal(false);

    QVBoxLayout *layout = new QVBoxLayout(chartDialog);
    layout->addWidget(chartView);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("关闭", chartDialog);
    QPushButton *saveButton  = new QPushButton("保存图片", chartDialog);
    connect(closeButton, &QPushButton::clicked, chartDialog, &QDialog::accept);
    connect(saveButton,  &QPushButton::clicked, this, [chartView, column]() {
        const QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                              "保存图表",
                                                              QDir::homePath() + "/" + column + "_chart.png",
                                                              "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg)");
        if (!fileName.isEmpty()) {
            const QPixmap pixmap = chartView->grab();
            pixmap.save(fileName);
        }
    });
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    chartDialog->resize(900, 600);
    chartDialog->show();
    chartDialog->setAttribute(Qt::WA_DeleteOnClose);
}

void DeviceDialog::on_cautionButton_clicked()
{
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "创建工单",
                              "检测到设备异常，是否立即创建维修工单？",
                              QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    const QString currentUser = "服务端监控人员";
    const QString description = "设备参数异常，需要检修";

    QJsonObject obj;
    obj["temperature"] = deviceParams.temperature;
    obj["pressure"]    = deviceParams.pressure;
    obj["vibration"]   = deviceParams.vibration;
    obj["current"]     = deviceParams.current;
    obj["voltage"]     = deviceParams.voltage;
    obj["speed"]       = deviceParams.speed;
    const QString deviceParamsJson = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    QString priority = "中";
    if (deviceParams.temperature > 80.0 || deviceParams.pressure > 140.0 || deviceParams.vibration > 5.0)
        priority = "高";
    else if (deviceParams.temperature > 70.0 || deviceParams.pressure > 130.0 || deviceParams.vibration > 4.0)
        priority = "中";
    else
        priority = "低";

    QSqlQuery query;
    query.prepare("INSERT INTO tickets "
                  "(title, description, priority, status, creator, expert_username, "
                  " created_time, updated_time, device_params) "
                  "VALUES (:title, :desc, :priority, :status, :creator, :expert, "
                  " :created, :updated, :device_params)");
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
        const int ticketId = query.lastInsertId().toInt();
        QMessageBox::information(this, "成功",
                                 QString("工单创建成功！\n工单编号: %1\n优先级: %2")
                                     .arg(ticketId).arg(priority));
        emit workOrderCreated(ticketId, "设备异常维修工单");
    } else {
        QMessageBox::critical(this, "错误",
                              QString("工单创建失败: %1").arg(query.lastError().text()));
    }
}
void DeviceDialog::setupUI()
{
    // 设置对话框基本属性
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setFixedSize(2000, 1400); // 适应2/3屏幕区域

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建标题标签
    QLabel *titleLabel = new QLabel("设备参数实时监控", this);
    titleLabel->setStyleSheet("QLabel { font-size: 20px; font-weight: bold; color: #2c3e50; padding: 10px; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 创建设备状态网格布局
    QGroupBox *statusGroup = new QGroupBox("", this);
    statusGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    QGridLayout *statusLayout = new QGridLayout(statusGroup);

    // 创建状态标签
    QLabel *labels[] = {
        new QLabel("温度:", statusGroup), new QLabel("压力:", statusGroup),
        new QLabel("振动:", statusGroup), new QLabel("电流:", statusGroup),
        new QLabel("电压:", statusGroup), new QLabel("转速:", statusGroup),
        new QLabel("状态:", statusGroup), new QLabel("更新时间:", statusGroup)
    };

    // 创建显示数值的标签
    ui->tempLabel = new QLabel("0.0 °C", statusGroup);
    ui->pressureLabel = new QLabel("0.0 kPa", statusGroup);
    ui->vibrationLabel = new QLabel("0.0 mm/s", statusGroup);
    ui->currentLabel = new QLabel("0.0 A", statusGroup);
    ui->voltageLabel = new QLabel("0.0 V", statusGroup);
    ui->speedLabel = new QLabel("0 RPM", statusGroup);
    ui->statusLabel = new QLabel("停止", statusGroup);
    ui->updateTimeLabel = new QLabel("-", statusGroup);

    // 设置数值标签样式
    QString valueStyle = "QLabel { font-weight: bold; font-size: 13px; min-width: 100px; }";
    ui->tempLabel->setStyleSheet(valueStyle);
    ui->pressureLabel->setStyleSheet(valueStyle);
    ui->vibrationLabel->setStyleSheet(valueStyle);
    ui->currentLabel->setStyleSheet(valueStyle);
    ui->voltageLabel->setStyleSheet(valueStyle);
    ui->speedLabel->setStyleSheet(valueStyle);
    ui->statusLabel->setStyleSheet(valueStyle + "color: red;");
    ui->updateTimeLabel->setStyleSheet(valueStyle);

    // 添加到网格布局
    for (int i = 0; i < 8; ++i) {
        statusLayout->addWidget(labels[i], i % 4, (i / 4) * 2);
        statusLayout->addWidget(getValueLabel(i), i % 4, (i / 4) * 2 + 1);
    }

    mainLayout->addWidget(statusGroup);

    // 创建功能按钮区域
    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // 创建按钮
    ui->pushButton = new QPushButton("生成趋势图", buttonWidget);
    ui->pushButton_2 = new QPushButton("刷新历史", buttonWidget);
    ui->deleteHistoryButton = new QPushButton("清空历史", buttonWidget);
    ui->cautionButton = new QPushButton("创建工单", buttonWidget);
    ui->closeButton = new QPushButton("关闭", buttonWidget);

    // 设置按钮样式
    QString buttonStyle = "QPushButton { padding: 8px 16px; border-radius: 4px; }";
    ui->pushButton->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");
    ui->pushButton_2->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    ui->deleteHistoryButton->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    ui->cautionButton->setStyleSheet(buttonStyle + "background-color: #f39c12; color: white;");
    ui->closeButton->setStyleSheet(buttonStyle + "background-color: #7f8c8d; color: white;");

    // 添加到按钮布局
    buttonLayout->addWidget(ui->pushButton);
    buttonLayout->addWidget(ui->pushButton_2);
    buttonLayout->addWidget(ui->deleteHistoryButton);
    buttonLayout->addWidget(ui->cautionButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(ui->closeButton);

    mainLayout->addWidget(buttonWidget);

    // 创建组合框用于选择图表列
    QWidget *comboWidget = new QWidget(this);
    QHBoxLayout *comboLayout = new QHBoxLayout(comboWidget);
    comboLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *comboLabel = new QLabel("选择图表数据:", comboWidget);
    ui->comboBox = new QComboBox(comboWidget);
    ui->comboBox->addItems({"温度", "压力", "振动", "电流", "电压", "转速"});

    comboLayout->addWidget(comboLabel);
    comboLayout->addWidget(ui->comboBox);
    comboLayout->addStretch();

    mainLayout->addWidget(comboWidget);

    // 创建历史数据表格
    ui->tableHistoryView = new QTableView(this);
    ui->tableHistoryView->setStyleSheet(
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
        );
    ui->tableHistoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableHistoryView->setAlternatingRowColors(true);

    mainLayout->addWidget(ui->tableHistoryView, 1); // 表格占据剩余空间

    // 连接信号槽
    connect(ui->pushButton, &QPushButton::clicked, this, &DeviceDialog::on_pushButton_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DeviceDialog::on_pushButton_2_clicked);
    connect(ui->deleteHistoryButton, &QPushButton::clicked, this, &DeviceDialog::on_deleteHistoryButton_clicked);
    connect(ui->cautionButton, &QPushButton::clicked, this, &DeviceDialog::on_cautionButton_clicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &DeviceDialog::on_closeButton_clicked);
}

// 辅助函数：获取对应的数值标签
QLabel* DeviceDialog::getValueLabel(int index)
{
    switch (index) {
    case 0: return ui->tempLabel;
    case 1: return ui->pressureLabel;
    case 2: return ui->vibrationLabel;
    case 3: return ui->currentLabel;
    case 4: return ui->voltageLabel;
    case 5: return ui->speedLabel;
    case 6: return ui->statusLabel;
    case 7: return ui->updateTimeLabel;
    default: return nullptr;
    }
}
