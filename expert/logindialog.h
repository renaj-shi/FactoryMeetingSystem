#pragma once
#include <QDialog>
class QLineEdit;
class QPushButton;
class QLabel;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent=nullptr);
    void setBusy(bool busy);
    void setStatus(const QString& text, bool isError=false);

signals:
    void loginRequested(const QString& user, const QString& pwdPlain);
    void registerRequested(const QString& user, const QString& pwdPlain);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit* userEdit_{nullptr};
    QLineEdit* pwdEdit_{nullptr};
    QPushButton* loginBtn_{nullptr};
    QPushButton* registerBtn_{nullptr};
    QPushButton* cancelBtn_{nullptr};
    QLabel* statusLabel_{nullptr};
};
