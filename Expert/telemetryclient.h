#pragma once
#include <QObject>
#include <QJsonObject>

class TcpClient;

class TelemetryClient : public QObject {
    Q_OBJECT
public:
    explicit TelemetryClient(TcpClient* tcp, QObject* parent=nullptr);
    void subscribe(const QString& orderId);   // 发送 TELE|SUB|orderId
    void unsubscribe();                       // 发送 TELE|UNSUB|orderId
    QString currentOrder() const { return orderId_; }

signals:
    void metricsUpdated(const QString& orderId, const QJsonObject& metrics); // 包含 temperature/pressure 等
    void logAppended(const QString& orderId, const QString& line);
    void faultRaised(const QString& orderId, const QString& code, const QString& text, const QString& level);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
    QString orderId_;
};
