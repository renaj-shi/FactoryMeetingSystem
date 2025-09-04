#ifndef WORKORDERLISTDIALOG_H
#define WORKORDERLISTDIALOG_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QTcpSocket>
#include <QJsonArray>
#include <QMessageBox>

namespace Ui {
class WorkOrderListDialog;
}

class WorkOrderListDialog : public QWidget
{
    Q_OBJECT

public:
    explicit WorkOrderListDialog(QWidget *parent = nullptr);
    ~WorkOrderListDialog();

    // 设置连接信息
    void setConnection(QTcpSocket* socket, const QString& username);
    
    // 刷新工单列表
    void refresh();
    
    // 获取工单号列表
    QStringList getWorkOrderIds();

signals:
    // 工单列表更新时发出信号
    void ticketListUpdated(const QList<QPair<int, QString>>& orders);
    
    // 当选中的工单变化时发出信号
    void currentTicketChanged(int id);

public slots:
    // 处理从服务器收到的工单数据
    void handleTicketData(const QString& line);

private slots:
    // 按钮点击事件
    void onRefreshClicked();
    void onLoadLogsClicked();
    void onViewDetailsClicked();

    // 工单列表项选中变化
    void onTicketSelected(const QModelIndex& index);
    
    // 状态列按钮点击事件
    void onStatusButtonClicked();
    void onStatusButtonClicked(const QModelIndex& index);
    
    // 表格点击事件的统一处理函数
    void handleTableClicked(const QModelIndex& index);

private:
    Ui::WorkOrderListDialog *ui;
    QTcpSocket* m_socket; // 与服务器的连接
    QString m_username;   // 当前用户名
    
    // 工单列表模型
    QStandardItemModel* m_ticketModel;
    // 当前日志数据（用于弹出窗口）
    QJsonArray m_currentLogsData;
    // 存储所有工单的完整数据
    QJsonArray m_allTicketsData;
    
    // 当前选中的工单ID
    int m_selectedTicketId;
    // 保存待处理的工单行索引，用于处理服务器响应
    int m_pendingTicketRow;
    
    // 发送获取工单列表请求
    void requestTicketList(int page = 1, int pageSize = 50);
    
    // 发送获取设备日志请求
    void requestDeviceLogs(int ticketId, int page = 1, int pageSize = 100);
    
    // 解析并显示工单列表
    void displayTicketList(const QJsonArray& rows);
    
    // 解析并显示设备日志（现在会弹出对话框）
    void displayDeviceLogs(int ticketId, const QJsonArray& rows);
    
    // 解析并显示故障信息
    void displayFaultInfo(int ticketId, const QJsonArray& rows);
    
    // 更新按钮状态（根据是否选中工单）
    void updateButtonStates();
    
    // 弹出设备日志对话框
    void showDeviceLogsDialog(const QJsonArray& logsData);
};

#endif // WORKORDERLISTDIALOG_H