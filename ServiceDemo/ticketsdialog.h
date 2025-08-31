#ifndef TICKETSDIALOG_H
#define TICKETSDIALOG_H

#include <QDialog>
#include <QSqlQueryModel>

QT_BEGIN_NAMESPACE
namespace Ui { class TicketsDialog; }
QT_END_NAMESPACE

class TicketsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TicketsDialog(QWidget *parent = nullptr);
    ~TicketsDialog();

private slots:
    void refreshTable();   // 手动刷新槽
    void on_pushButton_clicked();
    void on_refreshButton_clicked();
    void on_deleteButton_clicked();
    void changeTickets();
    void on_changeButton_clicked();

private:
    Ui::TicketsDialog *ui;
    QSqlQueryModel *model;
};

#endif // TICKETSDIALOG_H
