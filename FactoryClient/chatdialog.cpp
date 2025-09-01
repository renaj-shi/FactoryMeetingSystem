#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QMessageBox>
#include <QDateTime>

ChatDialog::ChatDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
    
    // 连接按钮信号和槽
    connect(ui->joinChatButton, &QPushButton::clicked, this, &ChatDialog::on_joinChatButton_clicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatDialog::on_sendButton_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &ChatDialog::on_backButton_clicked);
    
    // 设置聊天面板默认隐藏
    ui->chatPanel->hide();
    
    // 默认隐藏返回按钮
    ui->backButton->hide();
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::setConnection(QTcpSocket* socket, const QString& username)
{
    m_socket = socket;
    m_username = username;
}

void ChatDialog::showChatSuccess(const QString& workOrderId)
{
    m_currentWorkOrderId = workOrderId;
    
    // 更新标题显示当前聊天的工单
    ui->titleLabel->setText(QString("聊天房间 (工单: %1)").arg(workOrderId));
    
    // 隐藏加入面板，将垂直间隔器设置为不占用空间，显示聊天面板
    ui->joinPanel->hide();
    ui->verticalSpacer->changeSize(20, 40, QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->chatPanel->show();
    ui->chatPanel->setEnabled(true);
    
    // 显示返回按钮
    ui->backButton->show();
    
    // 清空聊天历史
    ui->chatHistory->clear();
    
    // 添加系统消息
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->chatHistory->append(QString("[%1] 系统: 已成功加入工单 %2 的聊天房间").arg(timestamp, workOrderId));
}

void ChatDialog::showChatError(const QString& errorMessage)
{
    // 显示错误消息
    QMessageBox::warning(this, "加入失败", errorMessage);
    
    // 确保加入面板可见，将垂直间隔器设置为可扩展
    ui->joinPanel->show();
    ui->verticalSpacer->changeSize(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->chatPanel->hide();
    ui->chatPanel->setEnabled(false);
    
    // 隐藏返回按钮
    ui->backButton->hide();
}

void ChatDialog::appendMessage(const QString& sender, const QString& message, bool isSelf)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formattedMessage;
    
    if (isSelf) {
        formattedMessage = QString("[%1] 我: %2").arg(timestamp, message);
        ui->chatHistory->append(QString("<span style='color: #3b82f6;'>%1</span>").arg(formattedMessage));
    } else {
        formattedMessage = QString("[%1] %2: %3").arg(timestamp, sender, message);
        ui->chatHistory->append(formattedMessage);
    }
    
    // 自动滚动到底部
    QTextCursor cursor = ui->chatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->chatHistory->setTextCursor(cursor);
}

void ChatDialog::on_joinChatButton_clicked()
{
    QString workOrderId = ui->workOrderInput->text().trimmed();
    
    if (workOrderId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入工单号");
        return;
    }
    
    // 发送加入聊天请求信号
    emit joinChatRequested(workOrderId);
}

void ChatDialog::on_sendButton_clicked()
{
    QString message = ui->messageInput->toPlainText().trimmed();
    
    if (message.isEmpty()) {
        return;
    }
    
    if (m_currentWorkOrderId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先加入聊天房间");
        return;
    }
    
    // 发送消息信号
    emit messageSent(m_currentWorkOrderId, message);
    
    // 清空输入框
    ui->messageInput->clear();
}

void ChatDialog::on_backButton_clicked()
{
    // 隐藏聊天面板，显示加入面板，将垂直间隔器设置为可扩展
    ui->chatPanel->hide();
    ui->joinPanel->show();
    ui->verticalSpacer->changeSize(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 清空工单号输入
    ui->workOrderInput->clear();
    
    // 更新标题
    ui->titleLabel->setText("聊天房间");
    
    // 清空当前工单号
    m_currentWorkOrderId.clear();
    
    // 隐藏返回按钮
    ui->backButton->hide();
}