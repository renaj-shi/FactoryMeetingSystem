#include "meetingroomdialog.h"
#include "ui_meetingroomdialog.h"
#include "audioprocessor.h"
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFileDialog>  
#include <QBuffer>       
#include <QImage>        
#include <QPixmap>

MeetingRoomDialog::MeetingRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MeetingRoomDialog),
    meetingServer(new QTcpServer(this))
{
    ui->setupUi(this);
    setWindowTitle("远程专家会议室");

    // 连接信号槽
    connect(meetingServer, &QTcpServer::newConnection, this, &MeetingRoomDialog::onNewConnection);
    connect(ui->sendImageButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_sendImageButton_clicked);

    // 设置界面初始状态
    ui->statusLabel->setText("会议室未启动");
    ui->clientCountLabel->setText("在线用户: 0");

    // 初始化按钮状态
    ui->closeMeetingButton->setEnabled(false);
    ui->sendButton->setEnabled(true);
    ui->broadcastButton->setEnabled(true);
    ui->kickButton->setEnabled(true);
}

MeetingRoomDialog::~MeetingRoomDialog()
{
    stopMeetingServer();
    delete ui;
}

bool MeetingRoomDialog::startMeetingServer(int port)
{
    qDebug() << "进入 startMeetingServer 方法";

    if (meetingServer->isListening()) {
        qDebug() << "服务器已在监听，关闭现有连接";
        meetingServer->close();
    }

    qDebug() << "尝试监听端口:" << port;

    if (!meetingServer->listen(QHostAddress::Any, port)) {
        QString error = meetingServer->errorString();
        qDebug() << "监听失败:" << error;
        QMessageBox::warning(this, "启动失败", QString("无法启动会议服务器:\n%1").arg(error));
        return false;
    }

    qDebug() << "服务器启动成功";
    ui->statusLabel->setText(QString("会议室运行中 - 端口: %1").arg(port));
    ui->startMeetingButton->setEnabled(false);
    ui->closeMeetingButton->setEnabled(true);

    // 直接调用方法，避免使用 invokeMethod
    addSystemMessage("会议室已启动，等待用户连接...");

    return true;
}

void MeetingRoomDialog::addSystemMessage(const QString& message)
{
    if (ui && ui->chatTextEdit) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString formattedMessage = QString("<font color='gray'>[%1] 系统: %2</font>")
                                       .arg(time).arg(message);
        ui->chatTextEdit->append(formattedMessage);

        // 自动滚动到底部
        QScrollBar *scrollbar = ui->chatTextEdit->verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
    }
}

void MeetingRoomDialog::stopMeetingServer()
{
    if (meetingServer && meetingServer->isListening()) {
        // 通知所有客户端会议室关闭
        QJsonObject closeMsg;
        closeMsg["type"] = "meeting_closed";
        closeMsg["message"] = "会议室已关闭";
        broadcastMessage(closeMsg);

        meetingServer->close();
    }

    // 断开所有客户端连接
    for (QTcpSocket* client : meetingClients) {
        if (client) {
            client->disconnectFromHost();
            client->deleteLater();
        }
    }
    meetingClients.clear();
    clientUsers.clear();
    clientTypes.clear();
    userSockets.clear();

    ui->statusLabel->setText("会议室已关闭");
    ui->clientCountLabel->setText("在线用户: 0");
    ui->startMeetingButton->setEnabled(true);
    ui->closeMeetingButton->setEnabled(false);
    ui->clientListWidget->clear();

    addSystemMessage("会议室已关闭");
}

void MeetingRoomDialog::onNewConnection()
{
    QTcpSocket *socket = meetingServer->nextPendingConnection();
    if (!socket) return;

    connect(socket, &QTcpSocket::readyRead, this, &MeetingRoomDialog::onClientReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MeetingRoomDialog::onClientDisconnected);

    meetingClients.insert(socket);
    ui->clientCountLabel->setText(QString("在线用户: %1").arg(meetingClients.size()));
    addSystemMessage("新用户连接");
}

void MeetingRoomDialog::onClientReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        QByteArray data = socket->readLine().trimmed();

        if (data.startsWith("JSON:")) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data.mid(5), &error);
            if (error.error == QJsonParseError::NoError && !doc.isNull()) {
                handleChatMessage(socket, doc.object());
            } else {
                qDebug() << "JSON解析错误:" << error.errorString();
            }
        }
        else if (data.startsWith("AUDIO:")) {
            handleAudioData(socket, data.mid(6));
        }
        else if (data.startsWith("IMAGE:")) {
            handleImageData(socket, data.mid(6));
        }
    }
}

void MeetingRoomDialog::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString username = clientUsers.value(socket, "未知用户");
    QString clientType = clientTypes.value(socket, "未知类型");

    meetingClients.remove(socket);
    clientUsers.remove(socket);
    clientTypes.remove(socket);
    userSockets.remove(username);

    ui->clientCountLabel->setText(QString("在线用户: %1").arg(meetingClients.size()));
    updateClientList();

    // 广播用户离开消息
    QJsonObject leaveMsg;
    leaveMsg["type"] = "user_left";
    leaveMsg["user"] = username;
    leaveMsg["user_type"] = clientType;
    leaveMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");
    broadcastMessage(leaveMsg);

    addSystemMessage(QString("用户 %1 (%2) 离开了会议室").arg(username).arg(clientType));

    socket->deleteLater();
}

void MeetingRoomDialog::handleChatMessage(QTcpSocket* socket, const QJsonObject& json)
{
    QString type = json["type"].toString();

    if (type == "join_meeting") {
        // 用户加入会议室
        QString username = json["user"].toString();
        QString userType = json["user_type"].toString();

        clientUsers[socket] = username;
        clientTypes[socket] = userType;
        userSockets[username] = socket;

        // 发送加入成功响应
        QJsonObject responseMsg;
        responseMsg["type"] = "meeting_joined";
        responseMsg["message"] = "成功加入会议室";
        QJsonDocument doc(responseMsg);
        socket->write("JSON:" + doc.toJson(QJsonDocument::Compact) + "\n");

        // 广播用户加入消息
        QJsonObject joinMsg;
        joinMsg["type"] = "user_joined";
        joinMsg["user"] = username;
        joinMsg["user_type"] = userType;
        joinMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");
        broadcastMessage(joinMsg, socket);

        updateClientList();
        addSystemMessage(QString("用户 %1 (%2) 加入了会议室").arg(username).arg(userType));
    }
    else if (type == "chat") {
        // 聊天消息
        QString message = json["message"].toString();
        QString timestamp = json["timestamp"].toString();
        QString user = json["user"].toString();

        // 广播给所有用户
        broadcastMessage(json, nullptr);
        addChatMessage(user, message, timestamp);
    }
        else if (type == "image_message") {
        // 图片消息
        QString timestamp = json["timestamp"].toString();
        QString user = json["user"].toString();
        QString imageData = json["image_data"].toString();

        QString formattedMessage = QString("<br><b>%1</b>: <img src='data:image/png;base64,%2' style='max-width:200px;max-height:200px;' /><br>")
                               .arg(user).arg(imageData);
    ui->chatTextEdit->insertHtml(formattedMessage);
        
        // 记录系统消息
        addSystemMessage(QString("用户 %1 发送了一张图片").arg(user));
        
        // 广播给所有用户
        broadcastMessage(json, nullptr);
    }
    else if (type == "leave_meeting") {
        // 用户离开会议室
        socket->disconnectFromHost();
    }
}


void MeetingRoomDialog::handleImageData(QTcpSocket* socket, const QByteArray& data)
{
    QString username = clientUsers.value(socket, "未知用户");
    broadcastToAll(data, "IMAGE:", socket);
    addSystemMessage(QString("%1 发送了图片消息").arg(username));
}

void MeetingRoomDialog::broadcastMessage(const QJsonObject& json, QTcpSocket* exclude)
{
    QJsonDocument doc(json);
    QByteArray data = "JSON:" + doc.toJson(QJsonDocument::Compact) + "\n";
    broadcastToAll(data, "", exclude);
}

void MeetingRoomDialog::broadcastToAll(const QByteArray& data, const QString& type, QTcpSocket* exclude)
{
    QByteArray sendData = type.toUtf8() + data;

    for (QTcpSocket* client : meetingClients) {
        if (client && client != exclude && client->state() == QAbstractSocket::ConnectedState) {
            client->write(sendData);
        }
    }
}

void MeetingRoomDialog::updateClientList()
{
    ui->clientListWidget->clear();

    for (auto it = clientUsers.constBegin(); it != clientUsers.constEnd(); ++it) {
        QString username = it.value();
        QString type = clientTypes.value(it.key(), "未知");
        QString itemText = QString("%1 (%2)").arg(username).arg(type);

        QListWidgetItem *item = new QListWidgetItem(itemText, ui->clientListWidget);

        // 根据用户类型设置不同的颜色
        if (type == "factory") {
            item->setForeground(Qt::blue);
        } else if (type == "expert") {
            item->setForeground(Qt::green);
        }
    }
}

void MeetingRoomDialog::addChatMessage(const QString& user, const QString& message, const QString& time)
{
    QString formattedMessage;
    if (user == "系统") {
        formattedMessage = QString("<font color='gray'>[%1] %2: %3</font>")
        .arg(time).arg(user).arg(message);
    } else {
        formattedMessage = QString("[%1] <b>%2</b>: %3")
        .arg(time).arg(user).arg(message);
    }

    ui->chatTextEdit->append(formattedMessage);

    // 自动滚动到底部
    QScrollBar *scrollbar = ui->chatTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void MeetingRoomDialog::on_sendButton_clicked()
{
    QString message = ui->messageLineEdit->text().trimmed();
    if (message.isEmpty()) return;

    // 作为系统消息发送
    QJsonObject json;
    json["type"] = "chat";
    json["user"] = "系统管理员";
    json["message"] = message;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    broadcastMessage(json);
    addChatMessage("系统管理员", message, QDateTime::currentDateTime().toString("hh:mm:ss"));
    ui->messageLineEdit->clear();
}

void MeetingRoomDialog::on_broadcastButton_clicked()
{
    QString message = ui->messageLineEdit->text().trimmed();
    if (message.isEmpty()) return;

    // 广播系统通知
    QJsonObject json;
    json["type"] = "system_broadcast";
    json["message"] = message;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    broadcastMessage(json);
    addSystemMessage("系统广播: " + message);
    ui->messageLineEdit->clear();
}

void MeetingRoomDialog::on_kickButton_clicked()
{
    QListWidgetItem *item = ui->clientListWidget->currentItem();
    if (!item) return;

    QString userInfo = item->text();
    QString username = userInfo.split(" ").first();

    if (userSockets.contains(username)) {
        QTcpSocket *socket = userSockets[username];
        socket->disconnectFromHost();
        addSystemMessage(QString("已将用户 %1 移出会议室").arg(username));
    }
}

void MeetingRoomDialog::on_closeMeetingButton_clicked()
{
    stopMeetingServer();
    emit meetingClosed();
    close();
}

void MeetingRoomDialog::on_startMeetingButton_clicked()
{
    if (startMeetingServer(12347)) {
        qDebug() << "会议服务器启动成功";
    } else {
        qDebug() << "会议服务器启动失败";
    }
}

void MeetingRoomDialog::on_sendImageButton_clicked()
{
    // 打开文件选择对话框选择图片
    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", "", 
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    
    if (!imagePath.isEmpty()) {
        sendImageFromServer(imagePath);
    }
}

void MeetingRoomDialog::sendImageFromServer(const QString &imagePath)
{
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片");
        return;
    }

    // 限制图片大小
    QPixmap scaledPixmap = pixmap.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    scaledPixmap.save(&buffer, "PNG");

    // 转换为Base64编码
    QByteArray base64Data = byteArray.toBase64();

    // 创建图片消息
    QJsonObject json;
    json["type"] = "image_message";
    json["user"] = "系统管理员";
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");
    json["image_data"] = QString(base64Data);

    // 广播图片消息给所有客户端
    broadcastMessage(json);

    // 在服务端本地显示发送的图片
    QString formattedMessage = QString("<br><b>系统管理员</b>: <img src='data:image/png;base64,%1' style='max-width:200px;max-height:200px;' /><br>")
                              .arg(QString(base64Data));
    ui->chatTextEdit->insertHtml(formattedMessage);
    
    // 滚动到底部
    QScrollBar *scrollbar = ui->chatTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
    
    addSystemMessage("系统管理员发送了一张图片");
}

void MeetingRoomDialog::handleAudioData(QTcpSocket* socket, const QByteArray& data)
{
    QString username = clientUsers.value(socket, "未知用户");
    
    // 转发音频数据给其他客户端
    QByteArray audioData = QByteArray::fromBase64(data);
    
    // 创建音频消息
    QJsonObject json;
    json["type"] = "audio_message";
    json["user"] = username;
    json["audio_data"] = QString(data); // data已经是base64编码的
    
    // 广播给其他用户（除了发送者）
    broadcastMessage(json, socket);
    
    // 在服务端记录
}