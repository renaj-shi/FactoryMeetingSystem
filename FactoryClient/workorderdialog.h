#ifndef WORKORDERDIALOG_H
#define WORKORDERDIALOG_H

#include <QWidget>
#include <QButtonGroup>
#include <QLabel>

namespace Ui {
class WorkOrderDialog;
}

class WorkOrderDialog : public QWidget
{
    Q_OBJECT

public:
    explicit WorkOrderDialog(QWidget *parent = nullptr);
    ~WorkOrderDialog();

    // 获取工单标题
    QString getTitle() const;

    // 获取问题描述
    QString getDescription() const;

    // 获取优先级
    QString getPriority() const;

    // 设置结果标签文本
    void setResultText(const QString &text);

private slots:
    // 处理文本变化事件，更新字数统计
    void onTextChanged();

    // 创建工单按钮点击事件
    void onCreateTicketClicked();

signals:
    // 创建工单信号
    void createTicket();

private:
    Ui::WorkOrderDialog *ui;
    QButtonGroup *m_priorityGroup;
    QLabel *m_charCountLabel;
};

#endif // WORKORDERDIALOG_H