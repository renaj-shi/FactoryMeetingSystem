#pragma once
#include <QDialog>
#include <QJsonArray>

class QTableView;
class QStandardItemModel;
class QPushButton;
class QSpinBox;
class QLabel;

class TicketsClient;
class WorkOrderManager;

class TicketsViewDialog : public QDialog {
    Q_OBJECT
public:
    explicit TicketsViewDialog(QWidget* parent=nullptr);
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

    // 操作
    void onJoinSelected();
    void onLoadLogs();
    void onLoadFaults();
    void onExportLogs();
    void onExportFaults();
    void onLocateFromFault();            // 从选中故障定位日志
    void onSelectionChanged();           // 控制“再次加入”可用性

private:
    int selectedId() const;
    void exportModelToCsv(QStandardItemModel* m, const QString& defaultName);
    // 定位：先按 seq 精确匹配，找不到再按时间最近
    int findLogRowBySeq(int seq) const;
    int findNearestLogRowByTime(const QString& isoTs) const;
    qint64 parseIso(const QString& s) const;

    // 工单列表
    QTableView* table_{nullptr};
    QStandardItemModel* model_{nullptr};
    QPushButton* btnRefresh_{nullptr};
    QPushButton* btnJoin_{nullptr};

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
};
