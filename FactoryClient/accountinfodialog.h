#ifndef ACCOUNTINFODIALOG_H
#define ACCOUNTINFODIALOG_H

#include <QWidget>

namespace Ui {
class AccountInfoDialog;
}

class AccountInfoDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AccountInfoDialog(QWidget *parent = nullptr);
    ~AccountInfoDialog();

    void setUsername(const QString &username);

signals:
    void logoutRequested();
    void profileEditRequested();
    void passwordChangeRequested();

private slots:
    void on_logoutButton_clicked();
    void on_profileButton_clicked();
    void on_passwordButton_clicked();

private:
    Ui::AccountInfoDialog *ui;
};

#endif // ACCOUNTINFODIALOG_H