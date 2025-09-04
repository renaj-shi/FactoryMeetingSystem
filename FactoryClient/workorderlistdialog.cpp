#include "workorderlistdialog.h"
#include "ui_workorderlistdialog.h"
#include "workorderdetaildialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDialog>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QScrollArea>
#include <QDate>
#include <QBrush>
#include <QColor>
#include <QLatin1Char>

// 设备日志对话框类，支持显示全部日志或仅显示异常日志
class DeviceLogsDialog : public QDialog
{
public:
    DeviceLogsDialog(const QStringList& headers, const QJsonArray& logsData, bool showOnlyFaults = false, QWidget *parent = nullptr)
        : QDialog(parent),
          m_showOnlyFaults(showOnlyFaults)
    {
        setWindowTitle(showOnlyFaults ? "设备故障日志" : "设备日志详情");
        setMinimumSize(800, 600);
        resize(900, 700);
        
        // 创建主布局
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // 创建表格视图
        m_tableView = new QTableView(this);
        m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_tableView->setShowGrid(true);
        m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        
        // 存储原始日志数据
        m_allLogsData = logsData;
        
        // 初始化数据模型并显示日志
        m_model = new QStandardItemModel(this);
        m_model->setHorizontalHeaderLabels(headers);
        m_tableView->setModel(m_model);
        
        // 根据showOnlyFaults参数显示对应日志
        displayLogs(showOnlyFaults);
        
        mainLayout->addWidget(m_tableView);
        
        // 创建底部按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        
        // 添加切换按钮（仅在有故障日志时显示）
        if (hasFaultLogs()) {
            m_toggleButton = new QPushButton(showOnlyFaults ? "显示全部日志" : "仅显示故障日志", this);
            m_toggleButton->setMinimumSize(120, 30);
            connect(m_toggleButton, &QPushButton::clicked, this, &DeviceLogsDialog::toggleLogsDisplay);
            buttonLayout->addWidget(m_toggleButton);
            buttonLayout->addSpacing(10);
        }
        
        QPushButton* closeButton = new QPushButton("关闭", this);
        closeButton->setMinimumSize(80, 30);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        buttonLayout->addWidget(closeButton);
        
        mainLayout->addLayout(buttonLayout);
    }
    
private:
    QTableView* m_tableView;        // 表格视图
    QStandardItemModel* m_model;    // 数据模型
    QPushButton* m_toggleButton;    // 切换显示模式按钮
    QJsonArray m_allLogsData;       // 全部日志数据
    bool m_showOnlyFaults;          // 是否仅显示故障日志
    
    // 显示日志数据，根据参数决定是否只显示故障日志
    void displayLogs(bool showOnlyFaults)
    {
        m_model->removeRows(0, m_model->rowCount());
        
        for (const QJsonValue& value : m_allLogsData) {
            if (value.isObject()) {
                QJsonObject obj = value.toObject();
                
                // 如果设置了只显示故障日志且当前日志正常，则跳过
                if (showOnlyFaults && obj.value("status").toInt() == 1) {
                    continue;
                }
                
                QList<QStandardItem*> rowItems;
                
                rowItems << new QStandardItem(QString::number(obj.value("id").toInt()));
                rowItems << new QStandardItem(QString::number(obj.value("temperature").toDouble(), 'f', 1));
                rowItems << new QStandardItem(QString::number(obj.value("pressure").toDouble(), 'f', 1));
                rowItems << new QStandardItem(QString::number(obj.value("vibration").toDouble(), 'f', 2));
                rowItems << new QStandardItem(QString::number(obj.value("current").toDouble(), 'f', 1));
                rowItems << new QStandardItem(QString::number(obj.value("voltage").toDouble(), 'f', 1));
                rowItems << new QStandardItem(QString::number(obj.value("speed").toInt()));
                
                // 设置状态文本和样式
                QStandardItem* statusItem = new QStandardItem(obj.value("status").toInt() ? "正常" : "异常");
                if (!obj.value("status").toInt()) {
                    statusItem->setBackground(QBrush(QColor(255, 200, 200)));
                    statusItem->setForeground(QBrush(QColor(200, 0, 0)));
                }
                rowItems << statusItem;
                
                rowItems << new QStandardItem(obj.value("record_time").toString());
                
                m_model->appendRow(rowItems);
            }
}
        
        // 更新窗口标题
        setWindowTitle(showOnlyFaults ? "设备故障日志" : "设备日志详情");
        
        // 更新切换按钮文本
        if (m_toggleButton) {
            m_toggleButton->setText(showOnlyFaults ? "显示全部日志" : "仅显示故障日志");
        }
        
        m_showOnlyFaults = showOnlyFaults;
    }
    
    // 检查是否有故障日志
    bool hasFaultLogs()
    {
        for (const QJsonValue& value : m_allLogsData) {
            if (value.isObject() && !value.toObject().value("status").toInt()) {
                return true;
            }
        }
        return false;
    }
    
    // 切换日志显示模式
    void toggleLogsDisplay()
    {
        displayLogs(!m_showOnlyFaults);
    }
};

WorkOrderListDialog::WorkOrderListDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkOrderListDialog),
    m_socket(nullptr),
    m_allTicketsData(QJsonArray()),
    m_selectedTicketId(-1),
    m_pendingTicketRow(-1)
{
    ui->setupUi(this);

    // 设置QSplitter的初始大小
    QList<int> sizes; sizes << 300 << 600;
    ui->splitter->setSizes(sizes);
    
    // 初始化模型
    m_ticketModel = new QStandardItemModel(this);
    m_ticketModel->setHorizontalHeaderLabels(QStringList() << "工单编号" << "标题" << "优先级" << "状态" << "创建时间");
    ui->ticketTableView->setModel(m_ticketModel);
    ui->ticketTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 连接信号和槽
    connect(ui->refreshButton, &QPushButton::clicked, this, &WorkOrderListDialog::onRefreshClicked);
    connect(ui->logsButton, &QPushButton::clicked, this, &WorkOrderListDialog::onLoadLogsClicked);
    connect(ui->detailsButton, &QPushButton::clicked, this, &WorkOrderListDialog::onViewDetailsClicked);
    
    // 设置表格视图属性，使状态列可点击
    ui->ticketTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ticketTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ticketTableView->setStyleSheet("QTableView::item:hover { background-color: #f0f0f0; } \
                                       QTableView::item:selected { background-color: #3b82f6; color: white; } \
                                       QHeaderView::section { background-color: #f0f0f0; color: black; font-weight: bold; } ");
    
    // 先使用自定义的clicked信号处理函数，在内部处理选中和状态按钮点击事件
    connect(ui->ticketTableView, &QTableView::clicked, this, &WorkOrderListDialog::handleTableClicked);
    
    // 初始化按钮状态
    updateButtonStates();
    
    // 设置窗口标题
    this->setWindowTitle("工单列表");
}

// 更新按钮状态
void WorkOrderListDialog::updateButtonStates()
{
    bool hasSelection = (m_selectedTicketId > 0);
    
    // 使用样式表管理按钮状态，包含圆角效果
    QString enabledStyle = "background-color: #3b82f6; color: white; border: none; padding: 5px 10px; border-radius: 5px;";
    QString disabledStyle = "background-color: #d0d0d0; color: #808080; border: none; padding: 5px 10px; border-radius: 5px;";
    
    // 查看日志按钮根据是否选择工单启用/禁用
    if (hasSelection) {
        ui->logsButton->setStyleSheet(enabledStyle);
        ui->logsButton->setEnabled(true);
    } else {
        ui->logsButton->setStyleSheet(disabledStyle);
        ui->logsButton->setEnabled(false);
    }
    
    // refreshButton始终可用且保持蓝色
    ui->refreshButton->setStyleSheet(enabledStyle);
    ui->refreshButton->setEnabled(true);
    
    // detailsButton根据是否选择工单启用/禁用
    if (hasSelection) {
        ui->detailsButton->setStyleSheet(enabledStyle);
        ui->detailsButton->setEnabled(true);
    } else {
        ui->detailsButton->setStyleSheet(disabledStyle);
        ui->detailsButton->setEnabled(false);
    }
}

WorkOrderListDialog::~WorkOrderListDialog()
{
    delete ui;
}

void WorkOrderListDialog::setConnection(QTcpSocket* socket, const QString& username)
{
    m_socket = socket;
    m_username = username;
    
    // 刷新工单列表
    refresh();
}

void WorkOrderListDialog::refresh()
{
    // 重置选中工单的状态
    m_selectedTicketId = 0;
    updateButtonStates();
    
    if (m_socket && m_socket->isOpen()) {
        requestTicketList();
    } else {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
    }
}

void WorkOrderListDialog::onRefreshClicked()
{
    refresh();
}

void WorkOrderListDialog::onLoadLogsClicked()
{
    if (m_selectedTicketId > 0) {
        requestDeviceLogs(m_selectedTicketId);
    } else {
        QMessageBox::information(this, "提示", "请先选择一个工单");
    }
}

// 弹出设备日志对话框
void WorkOrderListDialog::showDeviceLogsDialog(const QJsonArray& logsData)
{
    QStringList headers = QStringList() << "ID" << "温度" << "压力" << "振动" << "电流" << "电压" << "转速" << "状态" << "时间";
    DeviceLogsDialog* dialog = new DeviceLogsDialog(headers, logsData, false, this);
    dialog->exec();
    delete dialog;
}

void WorkOrderListDialog::onViewDetailsClicked()
{
    if (m_selectedTicketId > 0) {
        // 查找选中工单的完整信息
        QJsonObject selectedTicket;
        for (const QJsonValue& value : m_allTicketsData) {
            if (value.isObject() && value.toObject().value("id").toInt() == m_selectedTicketId) {
                selectedTicket = value.toObject();
                break;
            }
        }
        
        // 提取工单信息
        QString ticketId = QString("%1").arg(selectedTicket.value("id").toInt());
        QString deviceName = selectedTicket.value("title").toString();
        QString description = selectedTicket.value("description").toString();
        
        // 优先级转换为中文
        QString priority = selectedTicket.value("priority").toString();
        if (priority.toLower() == "high" || priority == "高") {
            priority = "高";
        } else if (priority.toLower() == "medium" || priority == "中") {
            priority = "中";
        } else if (priority.toLower() == "low" || priority == "低") {
            priority = "低";
        }
        
        // 提交时间格式化
        QString submitTime = selectedTicket.value("created_time").toString();
        if (submitTime.contains('T')) {
            submitTime = submitTime.replace('T', ' ');
            if (submitTime.length() > 16) {
                submitTime = submitTime.left(16);
            }
        }
        
        // 状态转换为中文
        QString statusText;
        int status = selectedTicket.value("status").toInt();
        if (status == 0) {
            statusText = "待处理";
        } else if (status == 1) {
            statusText = "处理中";
        } else if (status == 2) {
            statusText = "已完成";
        } else {
            statusText = QString("状态%1").arg(status);
        }
        
        // 从服务端payload里拿真正的创建者/处理者
        QString creator = selectedTicket.value("creator").toString();
        QString expert  = selectedTicket.value("expert_username").toString();
        
        // 使用服务端的 updated_time 作为处理/完成时间
        QString updatedTime = selectedTicket.value("updated_time").toString();
        auto norm = [](QString t){
            if (t.contains('T')) { t.replace('T',' '); if (t.size()>16) t=t.left(16); }
            return t;
        };
        updatedTime = norm(updatedTime);
        
        // 创建操作记录数据
        QJsonArray operationRecords;
        
        // 1) 创建工单
        QJsonObject createRecord;
        createRecord["time"]      = submitTime;
        createRecord["operator"]  = creator.isEmpty() ? "-" : creator;  // 用 DB 的 creator
        createRecord["operation"] = "创建工单";
        createRecord["type"]      = "create";
        operationRecords.append(createRecord);
        
        // 2) 处理中（如果状态>=1）
        if (status >= 1) {
            QJsonObject processRecord;
            processRecord["time"]      = updatedTime.isEmpty() ? submitTime : updatedTime;
            processRecord["operator"]  = expert.isEmpty() ? creator : expert; // 使用专家账户名
            processRecord["operation"] = "处理工单";
            processRecord["type"]      = "process";
            operationRecords.append(processRecord);
        }
        
        // 3) 已完成（如果状态==2）
        if (status == 2) {
            QJsonObject completeRecord;
            completeRecord["time"]      = updatedTime.isEmpty() ? submitTime : updatedTime;
            completeRecord["operator"]  = m_username.isEmpty() ? "-" : m_username; // 使用当前工厂账户名
            completeRecord["operation"] = "确认完成";
            completeRecord["type"]      = "complete";
            operationRecords.append(completeRecord);
        }
        
        // 创建并显示工单详情对话框
        WorkOrderDetailDialog* dialog = new WorkOrderDetailDialog(this);
        dialog->setWorkOrderInfo(ticketId, deviceName, description, priority, submitTime, statusText);
        dialog->setOperationRecords(operationRecords);
        dialog->exec();
        delete dialog;
    } else {
        QMessageBox::information(this, "提示", "请先选择一个工单");
    }
}

void WorkOrderListDialog::onTicketSelected(const QModelIndex& index)
{
    if (index.isValid()) {
        QStandardItem* item = m_ticketModel->item(index.row(), 0);
        if (item) {
            // 工单编号就是数字，直接转换
        m_selectedTicketId = item->text().toInt();
            
            // 更新按钮状态
            updateButtonStates();
            
            // 发射选中工单变化的信号
            emit currentTicketChanged(m_selectedTicketId);
        }
    }
}

void WorkOrderListDialog::handleTableClicked(const QModelIndex& index)
{
    // 先处理行选择
    onTicketSelected(index);
    
    // 如果点击的是状态列（第3列，索引为3），则处理状态更新
    if (index.isValid() && index.column() == 3) {
        onStatusButtonClicked(index);
    }
}

void WorkOrderListDialog::onStatusButtonClicked(const QModelIndex& index)
{
    // 获取工单ID（第一列）
    int ticketId = m_ticketModel->data(m_ticketModel->index(index.row(), 0)).toInt();
    
    // 获取当前状态文本
    QString statusText = m_ticketModel->data(index).toString();
    
    // 处理待处理和处理中状态的工单
    if (statusText == "待处理" || statusText == "处理中") {
        // 弹出确认对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认完成", 
                                     QString("确定要将工单 #%1 标记为已完成吗？").arg(ticketId),
                                     QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // 发送请求到服务器更新状态
            if (m_socket && m_socket->isOpen()) {
                // 保存当前行索引，用于处理响应
                m_pendingTicketRow = index.row();
                
                // 发送工单完成请求
                QString request = QString("TICKET|COMPLETE|%1\n").arg(ticketId);
                m_socket->write(request.toUtf8());
                m_socket->flush();
                
                // 不显示等待消息，因为WorkOrderListDialog没有statusBar组件
            }
            else {
                QMessageBox::warning(this, "连接错误", "无法连接到服务器，请检查网络连接");
            }
        }
    }
    // 对于已完成的工单，不做任何操作
}

// 为了保持兼容性，保留无参数版本
void WorkOrderListDialog::onStatusButtonClicked()
{
    QModelIndex index = ui->ticketTableView->currentIndex();
    if (index.isValid()) {
        onStatusButtonClicked(index);
    }
}

void WorkOrderListDialog::requestTicketList(int page, int pageSize)
{
    if (m_socket && m_socket->isOpen()) {
        QString request = QString("TICKETS|LIST|%1|%2\n").arg(page).arg(pageSize);
        m_socket->write(request.toUtf8());
        m_socket->flush();
    }
}

void WorkOrderListDialog::requestDeviceLogs(int ticketId, int page, int pageSize)
{
    if (m_socket && m_socket->isOpen()) {
        QString request = QString("TICKETS|LOGS|%1|%2|%3\n").arg(ticketId).arg(page).arg(pageSize);
        m_socket->write(request.toUtf8());
        m_socket->flush();
    }
}

void WorkOrderListDialog::handleTicketData(const QString& line)
{
    QStringList parts = line.split('|');
    if (parts.size() < 3) return;
    
    QString type = parts[0];
    
    // 兼容专家端的格式：TICKETS|SUB|OK|payload
    if (type == "TICKETS" && parts.size() >= 4) {
        QString subType = parts[1];
        QString status = parts[2];
        
        if (status == "OK") {
            QString jsonData = parts.mid(3).join("|");
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);
            
            if (error.error == QJsonParseError::NoError) {
                if (subType == "LIST" && doc.isArray()) {
                    // 处理工单列表数据（专家端格式返回的是数组）
                    QJsonArray rows = doc.array();
                    displayTicketList(rows);
                } else if (subType == "LOGS" && parts.size() >= 4) {
                // 处理设备日志数据
                if (doc.isArray()) {
                    // 日志ID从请求中获取
                    displayDeviceLogs(m_selectedTicketId, doc.array());
                }
            }
            }
        }
    }
    // 保留原有格式的兼容处理，确保向后兼容
    else if (type == "TICKET") {
        if (parts[1] == "LIST" && parts.size() >= 3) {
            QString jsonData = parts.mid(2).join("|");
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);
            
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("rows") && obj["rows"].isArray()) {
                    QJsonArray rows = obj["rows"].toArray();
                    displayTicketList(rows);
                }
            }
        } else if (parts[1] == "LOGS" && parts.size() >= 4) {
            int ticketId = parts[2].toInt();
            QString jsonData = parts.mid(3).join("|");
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);
            
            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("rows") && obj["rows"].isArray()) {
                    QJsonArray rows = obj["rows"].toArray();
                    displayDeviceLogs(ticketId, rows);
                }
            }
        } else if (parts[1] == "COMPLETE") {
            // 处理工单完成响应
            if (parts.size() >= 3) {
                if (parts[2] == "OK") {
                    // 工单状态更新成功
                    if (m_pendingTicketRow >= 0 && m_pendingTicketRow < m_ticketModel->rowCount()) {
                        // 本地更新状态为已完成
                        QStandardItem* statusItem = m_ticketModel->item(m_pendingTicketRow, 3);
                        if (statusItem) {
                            statusItem->setText("已完成");
                            statusItem->setForeground(QBrush(QColor(0, 128, 0))); // 绿色
                        }
                        
                        // 显示成功消息
                        QMessageBox::information(this, "操作成功", "工单状态已更新为已完成");
                    }
                    
                    // 重置待处理行索引
                    m_pendingTicketRow = -1;
                } else if (parts[2] == "ERR" && parts.size() >= 4) {
                    // 工单状态更新失败
                    QString errorMsg = parts.mid(3).join("|");
                    QMessageBox::warning(this, "操作失败", "工单状态更新失败: " + errorMsg);
                    
                    // 重置待处理行索引
                    m_pendingTicketRow = -1;
                }
            }
        }
    }
}

void WorkOrderListDialog::displayTicketList(const QJsonArray& rows)
{
    m_ticketModel->removeRows(0, m_ticketModel->rowCount());
    m_allTicketsData = rows; // 保存完整的工单数据
    
    // 设置表格为不可编辑状态
    ui->ticketTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    for (int row = 0; row < rows.size(); ++row) {
        const QJsonValue& value = rows[row];
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            QList<QStandardItem*> rowItems;
            
            // 工单编号 - 直接显示服务端发来的数字
            QString ticketId = QString::number(obj.value("id").toInt());
            rowItems << new QStandardItem(ticketId);
            
            // 标题
            rowItems << new QStandardItem(obj.value("title").toString());
            
            // 优先级 - 将英文转换为中文并设置对应颜色
            QString priority = obj.value("priority").toString();
            QString priorityText;
            QStandardItem* priorityItem = new QStandardItem();
            
            if (priority.toLower() == "high" || priority == "高") {
                priorityText = "高";
                priorityItem->setForeground(QBrush(QColor(255, 0, 0))); // 红色
            } else if (priority.toLower() == "medium" || priority == "中") {
                priorityText = "中";
                priorityItem->setForeground(QBrush(QColor(255, 165, 0))); // 橙色
            } else if (priority.toLower() == "low" || priority == "低") {
                priorityText = "低";
                priorityItem->setForeground(QBrush(QColor(0, 0, 255))); // 蓝色
            } else {
                priorityText = priority; // 保留原始值
            }
            priorityItem->setText(priorityText);
            rowItems << priorityItem;
            
            // 状态列 - 创建按钮
            int status = obj.value("status").toInt();
            QString statusText;
            QStandardItem* statusItem = new QStandardItem();
            
            if (status == 0) {
                statusText = "待处理";
                statusItem->setText(statusText);
                // 为待处理状态的单元格设置用户数据，存储工单ID，以便点击时获取
                statusItem->setData(obj.value("id").toInt(), Qt::UserRole);
            } else if (status == 1) {
                statusText = "处理中";
                statusItem->setText(statusText);
            } else if (status == 2) {
                statusText = "已完成";
                statusItem->setText(statusText);
            } else {
                statusText = QString("状态%1").arg(status); // 默认值
                statusItem->setText(statusText);
            }
            
            // 设置状态列的颜色
            if (status == 0) {
                statusItem->setForeground(QBrush(QColor(255, 0, 0))); // 红色
                // 为待处理状态的单元格设置用户数据，存储工单ID，以便点击时获取
                statusItem->setData(obj.value("id").toInt(), Qt::UserRole);
            } else if (status == 1) {
                statusItem->setForeground(QBrush(QColor(255, 165, 0))); // 橙色
                // 为处理中状态的单元格也设置用户数据，存储工单ID
                statusItem->setData(obj.value("id").toInt(), Qt::UserRole);
            } else if (status == 2) {
                statusItem->setForeground(QBrush(QColor(0, 128, 0))); // 绿色
            }
            
            rowItems << statusItem;
            
            // 创建时间
            QString timeText = obj.value("created_time").toString();
            if (timeText.contains('T')) {
                timeText = timeText.replace('T', ' ');
                if (timeText.length() > 16) {
                    timeText = timeText.left(16);
                }
            }
            rowItems << new QStandardItem(timeText);
            
            m_ticketModel->appendRow(rowItems);
        }
    }
    
    // 创建工单列表数据并发出信号
    QList<QPair<int,QString>> orders;
    for (const QJsonValue& v : rows) {
        if (!v.isObject()) continue;
        const auto o = v.toObject();
        orders.push_back({ o.value("id").toInt(), o.value("title").toString() });
    }
    emit ticketListUpdated(orders);
}

void WorkOrderListDialog::displayDeviceLogs(int ticketId, const QJsonArray& rows)
{
    // 存储日志数据，然后弹出对话框显示
    m_currentLogsData = rows;
    showDeviceLogsDialog(rows);
}

QStringList WorkOrderListDialog::getWorkOrderIds()
{
    QStringList workOrderIds;
    
    // 遍历所有工单数据，提取工单号
    for (const QJsonValue& value : m_allTicketsData) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            // 获取工单号并转换为字符串
            QString ticketId = QString::number(obj.value("id").toInt());
            workOrderIds << ticketId;
        }
    }
    
    return workOrderIds;
}