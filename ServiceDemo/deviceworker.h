#ifndef DEVICEWORKER_H
#define DEVICEWORKER_H

#include <QObject>
#include <QTimer>
#include "mainwindow.h"   // 为了拿到 DeviceParams 定义

class DeviceWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeviceWorker(QObject *parent = nullptr);
    void start();          // 槽：线程开始运行
public slots:
    void stop();           // 槽：停止采集
signals:
    void newData(const DeviceParams &data);   // 把最新数据发出去
    void finished();                          // 线程结束信号

private slots:
    void onTimeout();      // 定时采集

private:
    QTimer *m_timer = nullptr;
};

#endif // DEVICEWORKER_H
