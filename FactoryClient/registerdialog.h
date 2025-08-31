#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

public slots:
    void onRegisterSuccess(const QString &message);
    void onRegisterError(const QString &message);

private slots:
    void on_registerConfirmButton_clicked();
    void on_backToLoginButton_clicked();
    void onTogglePasswordVisibility();
    void onToggleConfirmPasswordVisibility();

signals:
    void registerResult(bool success, const QString &message);
    void backToLogin();

private:
    Ui::RegisterDialog *ui;
    bool isPasswordVisible;
    bool isConfirmPasswordVisible;
};

#endif // REGISTERDIALOG_H