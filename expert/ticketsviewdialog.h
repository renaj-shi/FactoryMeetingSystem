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
    void onJoinSelected();
    void onRefresh();

private:
    QTableView* table_{nullptr};
    QStandardItemModel* model_{nullptr};
    QPushButton* btnRefresh_{nullptr};
    QPushButton* btnJoin_{nullptr};

    TicketsClient* cli_{nullptr};
    WorkOrderManager* orders_{nullptr};

    int selectedId() const;
};
