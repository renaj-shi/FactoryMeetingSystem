#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    
    // 设置连接信息
    void setConnection(QTcpSocket* socket, const QString& username);
    
    // 显示聊天成功后的界面
    void showChatSuccess(const QString& workOrderId);
    
    // 显示聊天失败信息
    void showChatError(const QString& errorMessage);
    
    // 添加消息到聊天记录
    void appendMessage(const QString& sender, const QString& message, bool isSelf = false);

signals:
    // 加入聊天信号
    void joinChatRequested(const QString& workOrderId);
    
    // 发送消息信号
    void messageSent(const QString& workOrderId, const QString& message);

private slots:
    // 加入聊天按钮点击事件
    void on_joinChatButton_clicked();
    
    // 发送按钮点击事件
    void on_sendButton_clicked();
    
    // 返回按钮点击事件
    void on_backButton_clicked();

private:
    Ui::ChatDialog *ui;
    QTcpSocket* m_socket = nullptr;
    QString m_username;
    QString m_currentWorkOrderId; // 当前聊天的工单号
};

#endif // CHATDIALOG_H