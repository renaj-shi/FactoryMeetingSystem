#pragma once
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>

class QTableView;
class QPlainTextEdit;
class QStandardItemModel;
class QPushButton;

class KnowledgeClient;

class KnowledgeViewDialog : public QDialog {
    Q_OBJECT
public:
    explicit KnowledgeViewDialog(QWidget* parent=nullptr);
    void bindServices(KnowledgeClient* cli);
    void refresh();

private slots:
    void onListed(const QJsonArray& rows);
    void onListFailed(const QString& reason);
    void onViewDetail();

private:
    int selectedId() const;

    QTableView* table_{nullptr};
    QStandardItemModel* model_{nullptr};
    QPlainTextEdit* detail_{nullptr};
    QPushButton* btnRefresh_{nullptr};
    QPushButton* btnDetail_{nullptr};

    KnowledgeClient* cli_{nullptr};
};
