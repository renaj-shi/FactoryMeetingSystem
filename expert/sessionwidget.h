#pragma once
#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
class QLineEdit; class QPushButton; class QLabel; class QPlainTextEdit;
class QJsonObject;
QT_END_NAMESPACE

namespace QtCharts { class QChartView; class QChart; class QLineSeries; class QDateTimeAxis; class QValueAxis; }

// 新增：聊天前向声明
class ChatClient;

class TcpClient;
class AuthManager;
class WorkOrderManager;
class TelemetryClient;

class SessionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SessionWidget(QWidget* parent=nullptr);

    void bindServices(TcpClient* tcp, AuthManager* auth, WorkOrderManager* orders, TelemetryClient* telem);

    // 新增：注入聊天模块
    void attachChat(ChatClient* chat);

    QString currentOrder() const { return currentOrderId_; }

public slots:
    void setOrderId(const QString& id);

private slots:
    // 工单
    void onJoinClicked();
    void onLeaveClicked();
    void onJoined(const QString& orderId);
    void onJoinFailed(const QString& reason);
    void onLeft(const QString& orderId);

    // 设备数据
    void onMetrics(const QString& orderId, const QJsonObject& metrics);
    void onLogLine(const QString& orderId, const QString& line);
    void onFault(const QString& orderId, const QString& code, const QString& text, const QString& level);

    // 聊天
    void onSendChat();
    void onChatMessage(const QString& orderId, const QString& from, const QString& text, qint64 ts);

private:
    void buildUi();
    void buildCharts();
    void updateUiState();
    void appendChatLine(const QString& line);
    void appendLogLine(const QString& line);
    void appendFaultLine(const QString& line);
    void appendPoint(qint64 ms, double temp, double press);

    // services
    TcpClient* tcp_{nullptr};
    AuthManager* auth_{nullptr};
    WorkOrderManager* orders_{nullptr};
    TelemetryClient* telem_{nullptr};
    ChatClient* chat_{nullptr};           // 新增：聊天服务指针

    // UI 省略其余已有成员...
    QLineEdit* orderEdit_{nullptr};
    QPushButton* joinBtn_{nullptr};
    QPushButton* leaveBtn_{nullptr};
    QLabel* orderStatusLabel_{nullptr};

    QtCharts::QChartView* chartView_{nullptr};
    QtCharts::QChart* chart_{nullptr};
    QtCharts::QLineSeries* tempSeries_{nullptr};
    QtCharts::QLineSeries* pressSeries_{nullptr};
    QtCharts::QDateTimeAxis* axisX_{nullptr};
    QtCharts::QValueAxis* axisY_{nullptr};
    QPlainTextEdit* logView_{nullptr};
    QPlainTextEdit* faultView_{nullptr};

    QPlainTextEdit* chatView_{nullptr};
    QLineEdit* chatEdit_{nullptr};
    QPushButton* chatSendBtn_{nullptr};

    QString currentOrderId_;
    int maxPoints_ = 120;
};
