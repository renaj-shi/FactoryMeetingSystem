#pragma once
#include <QWidget>

class QLabel;
class QPushButton;

// [ADD] 新增：主窗口欢迎页（浅色、无图片）
class WelcomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget* parent=nullptr);
    // [ADD] 更新连接状态显示
    void setConnectionText(const QString& text);

signals:
    // [ADD] “立即登录”按钮
    void loginClicked();

private:
    QLabel* title_{nullptr};
    QLabel* subtitle_{nullptr};
    QLabel* connLabel_{nullptr};
    QPushButton* loginBtn_{nullptr};
};
