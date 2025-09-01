#pragma once
#include <QDialog>
#include <QJsonArray>

class QTableView;
class QStandardItemModel;
class QPushButton;

class TicketsClient;
class WorkOrderManager;

class TicketsViewDialog : public QDialog {
    Q_OBJECT
public:
    explicit TicketsViewDialog(QWidget* parent=nullptr);
    void bindServices(TicketsClient* cli, WorkOrderManager* orders);
    void refresh();

private slots:
    void onListed(const QJsonArray& rows);
    void onListFailed(const QString& reason);
    void onLogsListed(int ticketId, const QJsonArray& rows);
    void onLogsFailed(const QString& reason);
    void onFaultsListed(int ticketId, const QJsonArray& rows);
    void onFaultsFailed(const QString& reason);

    void onJoinSelected();
    void onLoadLogs();
    void onLoadFaults();

private:
    int selectedId() const;

    // UI
    QTableView* table_{nullptr};
    QStandardItemModel* model_{nullptr};
    QPushButton* btnRefresh_{nullptr};
    QPushButton* btnJoin_{nullptr};
    QPushButton* btnLogs_{nullptr};
    QPushButton* btnFaults_{nullptr};

    QTableView* logsView_{nullptr};
    QStandardItemModel* logsModel_{nullptr};

    QTableView* faultsView_{nullptr};
    QStandardItemModel* faultsModel_{nullptr};

    // services
    TicketsClient* cli_{nullptr};
    WorkOrderManager* orders_{nullptr};
};
