#include "deviceworker.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QThread>

DeviceWorker::DeviceWorker(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DeviceWorker::onTimeout);
}

void DeviceWorker::start()
{
    m_timer->start(2000);   // 每 2 秒采集一次
}

void DeviceWorker::stop()
{
    m_timer->stop();
    QString connName = QString("worker_%1").arg(quintptr(QThread::currentThreadId()));
    QSqlDatabase::removeDatabase(connName); // 清理线程专属连接
    emit finished();
}

static bool openThreadDB()
{
    // 每个线程用独立的连接名
    QString connName = QString("worker_%1").arg(quintptr(QThread::currentThreadId()));
    if (!QSqlDatabase::contains(connName)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName("industrial.db");
        if (!db.open()) {
            qDebug() << "worker thread open db fail:" << db.lastError().text();
            return false;
        }
    }
    return true;
}

void DeviceWorker::onTimeout()
{
    if (!openThreadDB())
        return;

    QString connName = QString("worker_%1").arg(quintptr(QThread::currentThreadId()));
    QSqlDatabase db = QSqlDatabase::database(connName);
    QSqlQuery q(db);
    // 与 DeviceDialog::generateRandomDeviceParams 相同算法
    auto gen = [](double mean, double dev) -> double {
        double u1 = QRandomGenerator::global()->generateDouble();
        double u2 = QRandomGenerator::global()->generateDouble();
        double z0 = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
        return mean + z0 * dev;
    };

    DeviceParams p;
    p.temperature = qBound(20.0, gen(45.0, 15.0), 100.0);
    p.pressure    = qBound(80.0, gen(100.0, 10.0), 150.0);
    p.vibration   = qBound(0.1, gen(2.0, 1.0), 6.0);
    p.current     = qBound(3.0, gen(10.0, 3.0), 20.0);
    p.voltage     = qBound(200.0, gen(220.0, 5.0), 240.0);
    p.speed       = qBound(1300, static_cast<int>(gen(1500, 100)), 1700);
    p.status      = (p.temperature <= 70 && p.pressure <= 130 && p.vibration <= 4);
    p.lastUpdate  = QDateTime::currentDateTime();

    q.prepare("INSERT INTO device_history "
              "(temperature, pressure, vibration, current, voltage, speed, status, record_time) "
              "VALUES (:t, :p, :v, :c, :volt, :spd, :st, CURRENT_TIMESTAMP)");

    q.bindValue(":t",   p.temperature);          // 温度
    q.bindValue(":p",   p.pressure);         // 压力
    q.bindValue(":v",   p.vibration);           // 振动
    q.bindValue(":c",   p.current);          // 电流
    q.bindValue(":volt", p.voltage);        // 电压
    q.bindValue(":spd",  p.speed);         // 转速
    q.bindValue(":st",   p.status);        // 状态
    if (!q.exec())
        qDebug() << "写 device_history 失败：" << q.lastError().text();
    else
        qDebug() << "已写入一条设备历史记录";
    emit newData(p);
}
