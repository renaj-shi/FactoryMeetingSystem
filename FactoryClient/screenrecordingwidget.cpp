#include "screenrecordingwidget.h"
#include "ui_screenrecordingwidget.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

ScreenRecordingWidget::ScreenRecordingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenRecordingWidget),
    m_recorder(new ScreenRecorder(this)),
    m_updateTimer(new QTimer(this)),
    m_recordSeconds(0)
{
    ui->setupUi(this);

    // 连接录制器信号
    connect(m_recorder, &ScreenRecorder::started, this, &ScreenRecordingWidget::onRecordStarted);
    connect(m_recorder, &ScreenRecorder::stopped, this, &ScreenRecordingWidget::onRecordStopped);
    connect(m_recorder, &ScreenRecorder::error, this, &ScreenRecordingWidget::onRecordError);
    connect(m_recorder, &ScreenRecorder::progressSec, this, &ScreenRecordingWidget::onRecordProgress);
    connect(m_recorder, &ScreenRecorder::info, this, &ScreenRecordingWidget::onRecordInfo);

    // 设置计时器更新录制时间显示
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        ui->recordingTimeLabel->setText(formatDuration(m_recordSeconds));
    });
    m_updateTimer->start(1000);

    // 初始化UI状态
    updateUIState(false);
    ui->outputPathLabel->setText("录制开始后显示保存路径");
    
    // 添加刷新工单按钮
    QPushButton *refreshButton = new QPushButton("刷新工单", this);
    refreshButton->setObjectName("refreshTicketsButton");
    refreshButton->setMinimumSize(QSize(100, 30));
    
    // 找到工单下拉框所在的布局，并在其后添加刷新按钮
    QGridLayout *grid = qobject_cast<QGridLayout*>(ui->recordingSettingsGroup->layout());
    if (grid && ui->workOrderComboBox) {
        int idx = grid->indexOf(ui->workOrderComboBox); // 找到下拉框在布局中的索引
        if (idx >= 0) {
            int r = 0, c = 0, rs = 1, cs = 1;
            grid->getItemPosition(idx, &r, &c, &rs, &cs); // 注意4个参数都要传地址
            // 放在下拉框右侧一列
            grid->addWidget(refreshButton, r, c + cs);
        } else {
            // 兜底：放在第一行最后一列
            int r = 0, c = qMax(0, grid->columnCount() - 1);
            grid->addWidget(refreshButton, r, c);
        }
    } else {
        // 再兜底：如果不是Grid或没找到控件，就加到groupBox里最末尾
        ui->recordingSettingsGroup->layout()->addWidget(refreshButton);
    }
    
    // 连接刷新按钮信号
    connect(refreshButton, &QPushButton::clicked, this, [this]{
        emit requestRefreshTickets();
        appendLog("正在刷新工单列表...");
    });
}

ScreenRecordingWidget::~ScreenRecordingWidget()
{
    // 确保录制停止
    if (m_recorder->isRecording()) {
        m_recorder->stop();
    }
    delete ui;
}

void ScreenRecordingWidget::initializeWorkOrderList(const QStringList &workOrderIds)
{
    ui->workOrderComboBox->clear();
    ui->workOrderComboBox->addItem("请选择工单", -1); // 提示项，id=-1
    
    // 保持兼容性，但内部使用addItem(text, id)模式
    for (const QString& idStr : workOrderIds) {
        ui->workOrderComboBox->addItem(idStr, idStr.toInt());
    }
}

void ScreenRecordingWidget::setWorkOrders(const QList<QPair<int,QString>>& orders)
{
    ui->workOrderComboBox->clear();
    ui->workOrderComboBox->addItem("请选择工单", -1); // 提示项，id=-1
    
    for (const auto& o : orders) {
        ui->workOrderComboBox->addItem(
            QString("%1 - %2").arg(o.first).arg(o.second), // 显示: "id - 标题"
            o.first // 用户数据: 真实id
        );
    }
    
    // 显示刷新成功的日志消息
    appendLog(QString("工单列表已更新，共 %1 个工单").arg(orders.size()));
}

void ScreenRecordingWidget::setWorkOrderId(int id)
{
    const int idx = ui->workOrderComboBox->findData(id);
    if (idx >= 0) {
        ui->workOrderComboBox->setCurrentIndex(idx);
    }
}

void ScreenRecordingWidget::on_startRecordButton_clicked()
{
    // 使用currentData获取真实ID
    const QVariant v = ui->workOrderComboBox->currentData();
    const int ticketId = v.isValid() ? v.toInt() : -1;
    
    if (ticketId <= 0) {
        QMessageBox::warning(this, "无法开始录制", "请从下拉框选择一个有效工单号");
        return;
    }

    // 获取录制设置
    bool includeAudio = ui->includeAudioCheckBox->isChecked();
    bool includeTimestamp = ui->includeTimestampCheckBox->isChecked();

    // 用真实id（数字）启动
    if (!m_recorder->start(QString::number(ticketId), includeTimestamp, includeAudio)) {
        appendLog("录制启动失败，请查看错误信息");
        return;
    }

    // 重置录制时间计数
    m_recordSeconds = 0;
    ui->recordingTimeLabel->setText("00:00:00");

    // 更新UI状态
    updateUIState(true);
    appendLog("正在准备录制...");
}

void ScreenRecordingWidget::on_stopRecordButton_clicked()
{
    if (m_recorder->isRecording()) {
        m_recorder->stop();
        appendLog("正在停止录制，请稍候...");
    }
}

void ScreenRecordingWidget::onRecordStarted(const QString &filePath)
{
    updateUIState(true);
    ui->outputPathLabel->setText(filePath);
    appendLog(QString("录制已开始，文件保存至：%1").arg(filePath));
}

void ScreenRecordingWidget::onRecordStopped(const QString &filePath)
{
    updateUIState(false);
    appendLog(QString("录制已停止，文件保存至：%1").arg(filePath));
    
    // 询问用户是否打开文件夹
    int reply = QMessageBox::question(this, "录制完成", 
                                     "屏幕录制已完成，是否打开保存文件夹？",
                                     QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QFileInfo fileInfo(filePath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    }
}

void ScreenRecordingWidget::onRecordError(const QString &msg)
{
    updateUIState(false);
    QMessageBox::warning(this, "录制错误", msg);
    appendLog(QString("错误: %1").arg(msg));
}

void ScreenRecordingWidget::onRecordProgress(int seconds)
{
    m_recordSeconds = seconds;
    ui->recordingTimeLabel->setText(formatDuration(seconds));
}

void ScreenRecordingWidget::onRecordInfo(const QString &msg)
{
    appendLog(msg);
}

void ScreenRecordingWidget::updateUIState(bool isRecording)
{
    ui->startRecordButton->setEnabled(!isRecording);
    ui->stopRecordButton->setEnabled(isRecording);
    ui->workOrderComboBox->setEnabled(!isRecording);
    ui->includeAudioCheckBox->setEnabled(!isRecording);
    ui->includeTimestampCheckBox->setEnabled(!isRecording);

    if (isRecording) {
        ui->recordingStatusLabel->setText("录制中");
        ui->recordingStatusLabel->setStyleSheet("color: #e74c3c;");
    } else {
        ui->recordingStatusLabel->setText("未录制");
        ui->recordingStatusLabel->setStyleSheet("color: #2ecc71;");
    }
}

QString ScreenRecordingWidget::formatDuration(int seconds)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    return QString("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'));
}

void ScreenRecordingWidget::appendLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    ui->logTextEdit->appendPlainText(QString("[%1] %2").arg(timestamp, message));
    
    // 滚动到底部
    QTextCursor cursor = ui->logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->logTextEdit->setTextCursor(cursor);
}