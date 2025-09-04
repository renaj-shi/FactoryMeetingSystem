#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include "screenrecorder.h"

namespace Ui { class ScreenRecordingWidget; }

class ScreenRecordingWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScreenRecordingWidget(QWidget *parent = nullptr);
    ~ScreenRecordingWidget();

    // 初始化工单号列表
    void initializeWorkOrderList(const QStringList &workOrderIds);
    
    // 设置当前工单号
    void setWorkOrderId(int id);
    
public slots:
    // 设置工单列表
    void setWorkOrders(const QList<QPair<int,QString>>& orders);

signals:
    // 请求刷新工单列表
    void requestRefreshTickets();

private slots:
    // UI事件处理
    void on_startRecordButton_clicked();
    void on_stopRecordButton_clicked();
    
    // 录制器信号处理
    void onRecordStarted(const QString &filePath);
    void onRecordStopped(const QString &filePath);
    void onRecordError(const QString &msg);
    void onRecordProgress(int seconds);
    void onRecordInfo(const QString &msg);

private:
    Ui::ScreenRecordingWidget *ui;
    ScreenRecorder *m_recorder;
    QTimer *m_updateTimer;
    int m_recordSeconds;

    // 更新UI状态
    void updateUIState(bool isRecording);
    // 格式化录制时间
    QString formatDuration(int seconds);
    // 记录日志
    void appendLog(const QString &message);
};