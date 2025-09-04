// ===================================================================
// [REVERT] 文件名: ticketspagewidget.h
// [REVERT] 内容: 回退此文件，移除了 onLogLocated() 槽和 locateTsAfterLoad_ 成员变量。
// ===================================================================
#pragma once

#include <QWidget>
#include <QJsonArray>

class QTableView;
class QStandardItemModel;
class QPushButton;
class QSpinBox;
class QLabel;

class TicketsClient;
class WorkOrderManager;

class TicketsPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit TicketsPageWidget(QWidget* parent=nullptr);
    void bindServices(TicketsClient* cli, WorkOrderManager* orders);
    void refresh();

private slots:
    // 服务端返回
    void onListed(const QJsonArray& rows);
    void onListFailed(const QString& reason);
    void onLogsListed(int ticketId, const QJsonArray& rows);
    void onLogsFailed(const QString& reason);
    void onFaultsListed(int ticketId, const QJsonArray& rows);
    void onFaultsFailed(const QString& reason);
    // [DEL] 移除了处理服务器定位响应的槽
    // void onLogLocated(int page, const QString& ts);
    // void onLocateLogFailed(const QString& reason);

    // 操作
    void onJoinSelected();
    void onLoadLogs();
    void onLoadFaults();
    void onExportLogs();
    void onExportFaults();
    void onLocateFromFault();
    void onSelectionChanged();

private:
    int selectedId() const;
    void exportModelToCsv(QStandardItemModel* m, const QString& defaultName);
    int findLogRowBySeq(int seq) const;
    int findNearestLogRowByTime(const QString& isoTs) const;
    qint64 parseIso(const QString& s) const;

    // 工单列表
    QTableView* table_{nullptr};
    QStandardItemModel* model_{nullptr};
    QPushButton* btnRefresh_{nullptr};
    QPushButton* btnJoin_{nullptr};

    // [ADD] 新增：用于在右侧详情面板显示工单信息的标签
    QLabel* detailPriorityLabel_{nullptr};
    QLabel* detailCreatorLabel_{nullptr};
    QLabel* detailLogsCountLabel_{nullptr};
    QLabel* detailFaultsCountLabel_{nullptr};
    QLabel* detailStatusLabel_{nullptr};
    QLabel* detailTimeLabel_{nullptr};

    // 日志区
    QTableView* logsView_{nullptr};
    QStandardItemModel* logsModel_{nullptr};
    QPushButton* btnLogs_{nullptr};
    QPushButton* btnExportLogs_{nullptr};
    QSpinBox*   logsPage_{nullptr};
    QSpinBox*   logsPageSize_{nullptr};
    QPushButton* logsPrev_{nullptr};
    QPushButton* logsNext_{nullptr};

    // 故障区
    QTableView* faultsView_{nullptr};
    QStandardItemModel* faultsModel_{nullptr};
    QPushButton* btnFaults_{nullptr};
    QPushButton* btnExportFaults_{nullptr};
    QPushButton* btnLocate_{nullptr};
    QSpinBox*   faultsPage_{nullptr};
    QSpinBox*   faultsPageSize_{nullptr};
    QPushButton* faultsPrev_{nullptr};
    QPushButton* faultsNext_{nullptr};

    // services
    TicketsClient*   cli_{nullptr};
    WorkOrderManager* orders_{nullptr};

    // 状态
    int currentTicketId_{-1};
    // [DEL] 移除了用于异步定位的状态变量
    // QString locateTsAfterLoad_;
};
