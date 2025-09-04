#pragma once
#include <QDialog>
class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent=nullptr);
    void setBusy(bool busy);
    void setStatus(const QString& text, bool isError=false);

    // [ADD] 新增：用于注册成功后回填账号
    void setUserAndPwd(const QString& user, const QString& pwd);

signals:
    void loginRequested(const QString& user, const QString& pwdPlain);
    void registerRequested(const QString& user, const QString& pwdPlain); // 保留，但本实现不再使用该信号
    // [ADD] 新增：请求打开注册对话框（由 MainWindow 处理）
    void openRegisterDialog();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    // [ADD] 显示/隐藏密码
    void onToggleShowPwd(bool on);

private:
    QLineEdit* userEdit_{nullptr};
    QLineEdit* pwdEdit_{nullptr};
    QPushButton* loginBtn_{nullptr};
    QPushButton* registerBtn_{nullptr};
    QPushButton* cancelBtn_{nullptr};
    QLabel* statusLabel_{nullptr};
    // [ADD] 显示密码选项
    QCheckBox* showPwd_{nullptr};
};
