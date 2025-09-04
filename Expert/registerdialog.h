#pragma once
#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;

// [ADD] 新增：独立注册对话框（浅色样式，无图片）
class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget* parent = nullptr);

    // [ADD] 设置忙碌状态（按钮禁用/启用）
    void setBusy(bool busy);

    // [ADD] 设置状态文本（支持错误样式）
    void setStatus(const QString& text, bool isError=false);

    // [ADD] 预填用户名（从登录页带入）
    void presetUsername(const QString& user);

signals:
    // [ADD] 发起注册请求（由外部连接到 AuthManager::registerUser）
    void registerRequested(const QString& user, const QString& pwdPlain);

private slots:
    // [ADD] 点击“注册”提交
    void onSubmit();
    // [ADD] 显示/隐藏密码
    void onToggleShowPwd(bool on);

private:
    QLineEdit* userEdit_{nullptr};
    QLineEdit* pwdEdit_{nullptr};
    QLineEdit* confirmEdit_{nullptr};
    QCheckBox* showPwd_{nullptr};
    QPushButton* okBtn_{nullptr};
    QPushButton* cancelBtn_{nullptr};
    QLabel* statusLabel_{nullptr};
};
