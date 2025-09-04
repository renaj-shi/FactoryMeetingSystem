#pragma once

#include <QWidget>
#include <QString>
#include <QJsonObject>
#include "chatmessagewidget.h"
#include "ffmpegrecorder.h"   // 新增：录屏类

class QPushButton;
namespace Ui { class SessionWidget; }
class TicketsPageWidget;
namespace QtCharts {
class QChartView; class QChart; class QLineSeries; class QDateTimeAxis; class QValueAxis;
}
class TcpClient;
class AuthManager;
class WorkOrderManager;
class TelemetryClient;
class ChatClient;
class TicketsClient;

class SessionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SessionWidget(QWidget* parent=nullptr);
    ~SessionWidget();
    
    void bindServices(TcpClient* tcp,
                      AuthManager* auth,
                      WorkOrderManager* orders,
                      TelemetryClient* telem,
                      TicketsClient* tickets);

    void attachChat(ChatClient* chat);

    QString currentOrder() const { return currentOrderId_; }
    
protected:
    void showEvent(QShowEvent* e) override;

signals:
    void createMeetingRequested();
    void joinMeetingRequested();

public slots:
    void setOrderId(const QString& id, const QString& title = QString());
    // 供 MainWindow 更新左侧连接状态
    void updateConnectionStatus(bool connected, const QString& text);

private slots:
    void onJoinClicked();
    void onLeaveClicked();
    void onJoined(const QString& orderId, const QString& title);
    void onJoinFailed(const QString& reason);
    void onLeft(const QString& orderId);
    void onMetrics(const QString& orderId, const QJsonObject& m);
    void onLogLine(const QString& orderId, const QString& line);
    void onFault(const QString& orderId, const QString& code, const QString& text, const QString& level);
    void onSendChat();
    void onChatMessage(const QString& orderId, const QString& from, const QString& text, qint64 ts);
    void onNavigationChanged();
    // 录屏相关
    void onRecordButtonClicked();
    void onRecordStarted(const QString& file);
    void onRecordStopped(const QString& file);
    void onRecordError(const QString& msg);
    void onRecordProgress(int sec);

private:
    void setupCharts();
    void setupConnections();
    void applyStyles();

    void updateUiState();
    void updateRecordUi(bool recording);
    void appendChatLine(const QString &sender, const QString &text, const QString &timestamp, ChatMessageWidget::Alignment alignment);
    void appendSystemMessage(const QString &text);
    void appendLogLine(const QString& line);
    void appendFaultLine(const QString& line);
    void refreshAxisRange();
    void clearChartData();
    QString makeMetricsLogLine(qint64 ts, const QJsonObject& m) const;
    QString formatLogLine(const QString& line) const;
    QString formatFaultLine(const QString& line) const;
    QIcon createSendIcon() const;
    QIcon createCameraIcon() const;
    QIcon createJoinIcon() const;
    QIcon createWrenchIcon() const;

    Ui::SessionWidget* ui;
    TicketsPageWidget* ticketsPage_{nullptr};
    TcpClient*       tcp_{nullptr};
    AuthManager*     auth_{nullptr};
    WorkOrderManager* orders_{nullptr};
    TelemetryClient* telem_{nullptr};
    ChatClient*      chat_{nullptr};
    QtCharts::QChartView*   chartView_{nullptr};
    QtCharts::QChart*       chart_{nullptr};
    QtCharts::QDateTimeAxis* axisX_{nullptr};
    QtCharts::QValueAxis*    axisY_{nullptr};
    QtCharts::QLineSeries* tempSeries_{nullptr};
    QtCharts::QLineSeries* pressSeries_{nullptr};
    QtCharts::QLineSeries* vibSeries_{nullptr};
    QtCharts::QLineSeries* currSeries_{nullptr};
    QtCharts::QLineSeries* voltSeries_{nullptr};
    QtCharts::QLineSeries* speedSeries_{nullptr};
    QString currentOrderId_;
    int maxPoints_ = 300;

    // 录屏成员
    FFmpegRecorder recorder_;
    QPushButton*   recordBtn_ = nullptr;
    int            recordSeconds_ = 0;
};
