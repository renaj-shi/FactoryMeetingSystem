#include "meetingroomdialog.h"
#include "ui_meetingroomdialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDataStream>
#include <QFileDialog>
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QElapsedTimer>
#include <QScrollArea>
#include <QGroupBox>
#include <QTimer>
MeetingRoomDialog::MeetingRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MeetingRoomDialog),
    meetingServer(new QTcpServer(this)),
    camera(nullptr),
    viewfinder(nullptr),
    imageCapture(nullptr),
    videoThread(nullptr)
{
    ui->setupUi(this);
    setupUI();
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

    // 连接视频按钮信号槽
    connect(ui->startVideoButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_startVideoButton_clicked);
    connect(ui->stopVideoButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_stopVideoButton_clicked);

    // 初始化视频框架
    initVideoFrames();

    updateUIStatus();
}

MeetingRoomDialog::~MeetingRoomDialog()
{
    stopMeetingServer();
    releaseCameraResources();
    delete ui;
}

void MeetingRoomDialog::initVideoFrames()
{
    // 清空现有的视频框架
    QLayoutItem* item;
    while ((item = ui->videoGridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 创建四个固定的视频框架
    for (int i = 0; i < 4; ++i) {
        QWidget* videoFrame = new QWidget();
        videoFrame->setMinimumSize(400, 300); // 设置最小尺寸
        videoFrame->setMaximumSize(500, 400); // 设置最大尺寸
        videoFrame->setStyleSheet("border: 2px solid #3498db; background-color: #ecf0f1;");

        QVBoxLayout* frameLayout = new QVBoxLayout(videoFrame);
        frameLayout->setContentsMargins(5, 5, 5, 5);
        frameLayout->setSpacing(0);

        QLabel* videoLabel = new QLabel("", videoFrame);
        videoLabel->setAlignment(Qt::AlignCenter);
        videoLabel->setStyleSheet("background-color: transparent;");
        videoLabel->setMinimumSize(390, 250); // 视频显示区域大小

        QLabel* infoLabel = new QLabel("空闲", videoFrame);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setStyleSheet("background-color: rgba(52, 152, 219, 0.7); color: white; font-size: 10px;");
        infoLabel->setFixedHeight(35);

        frameLayout->addWidget(videoLabel);
        frameLayout->addWidget(infoLabel);

        videoFrames.append(videoFrame);
        videoLabels.append(videoLabel);
        infoLabels.append(infoLabel);

        // 添加到网格布局
        int row = i / 2;
        int col = i % 2;
        ui->videoGridLayout->addWidget(videoFrame, row, col);

        // 设置行列的拉伸比例
        ui->videoGridLayout->setRowStretch(row, 1);
        ui->videoGridLayout->setColumnStretch(col, 1);
    }
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

    // 重置视频框架
    for (int i = 0; i < infoLabels.size(); ++i) {
        infoLabels[i]->setText("空闲");
    }

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

    // 释放对应的视频框架
    releaseVideoFrame(username);

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

        // 分配视频框架
        assignVideoFrame(username, userType);
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
    else if(type=="video_frame"){

        handleVideoFrame(json);

        QString fromUser   = json["user"].toString();
        QString base64Data = json["data"].toString();

        forwardVideoFrame(socket, fromUser, base64Data.toUtf8());
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
    else if (type == "video_frame") {
        handleVideoFrame(json);
    }
}

void MeetingRoomDialog::forwardVideoFrame(QTcpSocket *senderSocket,
                                          const QString &fromUser,
                                          const QByteArray &base64Jpeg)
{
    QJsonObject videoMsg;
    videoMsg["type"]   = "video_frame";
    videoMsg["data"]   = QString::fromLatin1(base64Jpeg);
    videoMsg["user"]   = fromUser;
    videoMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    QJsonDocument doc(videoMsg);
    QByteArray    payload = "JSON:" + doc.toJson(QJsonDocument::Compact) + "\n";

    qDebug() << "[Forward] 准备转发视频帧 from" << fromUser
             << "大小" << base64Jpeg.size() << "bytes";

    int sentCnt = 0;
    for (QTcpSocket *client : meetingClients) {
        if (client && client != senderSocket &&
            client->state() == QAbstractSocket::ConnectedState)
        {
            qint64 bytes = client->write(payload);
            qDebug() << "[Forward] 已转发" << bytes << "bytes 到"
                     << client->peerAddress().toString();
            ++sentCnt;
        }
    }
    qDebug() << "[Forward] 本次共转发给" << sentCnt << "个客户端";
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
    updateUIStatus();
}

void MeetingRoomDialog::releaseCameraResources()
{
#ifdef QT_MULTIMEDIA_LIB
    // 先停止视频线程
    if (videoThread) {
        videoThread->stop();
        videoThread->wait(1000);
        delete videoThread;
        videoThread = nullptr;
    }

    if (camera) {
        if (camera->state() == QCamera::ActiveState) {
            camera->stop();
        }
        delete camera;
        camera = nullptr;
    }

    if (viewfinder) {
        viewfinder->deleteLater();
        viewfinder = nullptr;
    }

    if (imageCapture) {
        delete imageCapture;
        imageCapture = nullptr;
    }

    // 释放服务器占用的视频框架
    if (infoLabels.size() > 0 && infoLabels[0]->text().startsWith("服务器")) {
        infoLabels[0]->setText("空闲");
        videoLabels[0]->setText("等待用户加入...");
        videoLabels[0]->clear();

        // 从userFrameMap中移除服务器
        QString serverUser;
        for (auto it = userFrameMap.constBegin(); it != userFrameMap.constEnd(); ++it) {
            if (it.value() == 0) {
                serverUser = it.key();
                break;
            }
        }
        if (!serverUser.isEmpty()) {
            userFrameMap.remove(serverUser);
        }
    }
#endif
}

void MeetingRoomDialog::updateUIStatus()
{
#ifdef QT_MULTIMEDIA_LIB
    const bool camActive = (camera && camera->state() == QCamera::ActiveState);
#else
    const bool camActive = false;
#endif

    // 视频按钮状态
    ui->startVideoButton->setEnabled(meetingServer->isListening() && !camActive);
    ui->stopVideoButton->setEnabled(camActive);
}

void MeetingRoomDialog::on_startVideoButton_clicked()
{
#ifdef QT_MULTIMEDIA_LIB
    releaseCameraResources();

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有找到可用的摄像头");
        return;
    }

    try {
        camera = new QCamera(cameras.first(), this);

        // 服务器固定使用第一个框架（索引0）
        int serverSlot = 0;
        if (infoLabels[serverSlot]->text() != "空闲") {
            // 如果第一个框架已被占用，先释放
            QString currentUser;
            for (auto it = userFrameMap.constBegin(); it != userFrameMap.constEnd(); ++it) {
                if (it.value() == serverSlot) {
                    currentUser = it.key();
                    break;
                }
            }
            if (!currentUser.isEmpty()) {
                releaseVideoFrame(currentUser);
            }
        }

        // 分配服务器框架
        infoLabels[serverSlot]->setText("服务器 (管理员)");
        videoLabels[serverSlot]->setText("本地摄像头");

        // 设置摄像头视图
        viewfinder = new QCameraViewfinder(videoLabels[serverSlot]);
        viewfinder->resize(videoLabels[serverSlot]->size());
        viewfinder->show();
        camera->setViewfinder(viewfinder);

        imageCapture = new QCameraImageCapture(camera, this);
        imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

        QCameraViewfinderSettings settings;
        camera->setViewfinderSettings(settings);

        // 发送路径：捕获 → sendVideoFrame()
        connect(imageCapture, &QCameraImageCapture::imageCaptured,
                this, [this](int, const QImage &image) {
                    this->sendVideoFrame(image);
                });

        camera->start();

        QElapsedTimer waitTimer;
        waitTimer.start();
        while (waitTimer.elapsed() < 3000) {
            if (camera->state() == QCamera::ActiveState) break;
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
        if (camera->state() != QCamera::ActiveState) {
            throw std::runtime_error("无法启动摄像头");
        }

        // 本地预览线程
        videoThread = new VideoThread(imageCapture, this);
        connect(videoThread, &VideoThread::frameCaptured, this, &MeetingRoomDialog::videoFrameReceived);
        videoThread->start();

        updateUIStatus();
        addSystemMessage("视频聊天已启动");

    } catch (const std::exception& e) {
        releaseCameraResources();
        QMessageBox::warning(this, "摄像头错误", QString("无法访问摄像头: %1").arg(e.what()));
    }
#else
    addSystemMessage("视频功能不可用");
#endif
}

void MeetingRoomDialog::on_stopVideoButton_clicked()
{
#ifdef QT_MULTIMEDIA_LIB
    releaseCameraResources();
    updateUIStatus();
    addSystemMessage("视频聊天已停止");
#else
    addSystemMessage("视频功能不可用");
#endif
}

void MeetingRoomDialog::videoFrameReceived(QImage image)
{
#ifdef QT_MULTIMEDIA_LIB
    int slot = 0;   // 本地固定用 0 号框
    if (slot < videoLabels.size()) {
        QPixmap pix = QPixmap::fromImage(image)
        .scaled(videoLabels[slot]->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
        videoLabels[slot]->setPixmap(pix);
    }
#else
    Q_UNUSED(image)
#endif
}

#ifdef QT_MULTIMEDIA_LIB
void MeetingRoomDialog::handleVideoFrame(const QJsonObject &json)
{
    QString username = json["user"].toString();
    QString base64Data = json["data"].toString();

    qDebug() << "收到视频帧 from:" << username << "数据长度:" << base64Data.length();

    // 特殊处理：如果是服务器自己的视频帧，直接返回
    if (username == "服务器") {
        qDebug() << "忽略服务器自己的视频帧";
        return;
    }
    if (username.isEmpty()) {
        qDebug() << "错误：收到的视频帧用户名为空，尝试从socket查找用户";

        // 尝试从当前连接的socket查找用户名
        QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
        if (senderSocket && clientUsers.contains(senderSocket)) {
            username = clientUsers[senderSocket];
            qDebug() << "从socket找到用户名:" << username;
        } else {
            qDebug() << "无法确定视频帧来源，丢弃该帧";
            return;
        }
    }
    // 查找对应用户的视频框架
    if (userFrameMap.contains(username)) {
        int frameIndex = userFrameMap[username];
        qDebug() << "找到用户框架位置:" << frameIndex << "for user:" << username;

        if (frameIndex < videoLabels.size()) {
            QByteArray imgData = QByteArray::fromBase64(base64Data.toUtf8());
            qDebug() << "解码后图像数据大小:" << imgData.size() << "bytes";

            QImage image;
            if (image.loadFromData(imgData, "JPEG")) {
                qDebug() << "成功加载图像, 尺寸:" << image.size();
                QPixmap pixmap = QPixmap::fromImage(image);
                videoLabels[frameIndex]->setPixmap(pixmap.scaled(
                    videoLabels[frameIndex]->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation));
                // 移除等待文本
                videoLabels[frameIndex]->setText("");
            } else {
                qDebug() << "加载图像失败";
                videoLabels[frameIndex]->setText("视频解码失败");
            }
        }
    } else {
        qDebug() << "用户" << username << "未分配视频框架，尝试分配...";
        // 如果用户没有分配框架，尝试分配框架
        QTcpSocket* userSocket = userSockets.value(username, nullptr);
        if (userSocket && clientTypes.contains(userSocket)) {
            QString userType = clientTypes[userSocket];
            assignVideoFrame(username, userType);

            qDebug() << "已为用户分配框架，延迟显示视频";
            // 延迟显示
            QTimer::singleShot(100, this, [this, username, base64Data]() {
                if (userFrameMap.contains(username)) {
                    int frameIndex = userFrameMap[username];
                    QByteArray imgData = QByteArray::fromBase64(base64Data.toUtf8());
                    QImage image;
                    if (image.loadFromData(imgData, "JPEG")) {
                        QPixmap pixmap = QPixmap::fromImage(image);
                        videoLabels[frameIndex]->setPixmap(pixmap.scaled(
                            videoLabels[frameIndex]->size(),
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation));
                        videoLabels[frameIndex]->setText("");
                    }
                }
            });
        } else {
            qDebug() << "无法找到用户socket或类型信息";
        }
    }
}
#endif

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
    json["audio_data"] = QString(data);

    // 广播给其他用户（除了发送者）
    broadcastMessage(json, socket);
}

void MeetingRoomDialog::setupUI()
{
    // 设置对话框基本属性
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setFixedSize(2000, 1500);

    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    // 左侧区域 - 视频会议区域
    QWidget *videoArea = new QWidget(this);
    videoArea->setMinimumWidth(1200); // 设置最小宽度
    QVBoxLayout *videoLayout = new QVBoxLayout(videoArea);
    videoLayout->setSpacing(10);

    // 视频网格布局容器 - 设置固定大小或使用拉伸因子
    QWidget *videoGridWidget = new QWidget(videoArea);
    videoGridWidget->setMinimumSize(1000, 800); // 设置视频区域的最小尺寸
    ui->videoGridLayout = new QGridLayout(videoGridWidget);
    ui->videoGridLayout->setSpacing(20); // 增加框架之间的间距
    ui->videoGridLayout->setContentsMargins(20, 20, 20, 20); // 设置边距

    // 设置视频网格布局的行列比例
    ui->videoGridLayout->setRowStretch(0, 1);
    ui->videoGridLayout->setRowStretch(1, 1);
    ui->videoGridLayout->setColumnStretch(0, 1);
    ui->videoGridLayout->setColumnStretch(1, 1);

    videoLayout->addWidget(videoGridWidget, 1); // 使用拉伸因子让视频区域占据更多空间

    // 视频控制按钮
    QWidget *videoControlWidget = new QWidget(this);
    QHBoxLayout *videoControlLayout = new QHBoxLayout(videoControlWidget);
    videoControlLayout->setContentsMargins(0, 0, 0, 0);

    ui->startVideoButton = new QPushButton("开启视频", videoControlWidget);
    ui->stopVideoButton = new QPushButton("停止视频", videoControlWidget);
    ui->sendImageButton = new QPushButton("发送图片", videoControlWidget);

    // 设置按钮样式
    QString buttonStyle = "QPushButton { padding: 8px 16px; border-radius: 4px; }";
    ui->startVideoButton->setStyleSheet(buttonStyle + "background-color: #3498db; color: white;");
    ui->stopVideoButton->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");
    ui->sendImageButton->setStyleSheet(buttonStyle + "background-color: #9b59b6; color: white;");
    // 设置按钮固定大小
    ui->startVideoButton->setFixedSize(120, 40);
    ui->stopVideoButton->setFixedSize(120, 40);
    ui->sendImageButton->setFixedSize(120, 40);

    videoControlLayout->addWidget(ui->startVideoButton);
    videoControlLayout->addWidget(ui->stopVideoButton);
    videoControlLayout->addWidget(ui->sendImageButton);
    videoControlLayout->addStretch();

    videoLayout->addWidget(videoControlWidget);

    // 右侧区域 - 聊天和控制面板
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(600);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(10);

    // 会议室状态
    QGroupBox *statusGroup = new QGroupBox("会议室状态", rightPanel);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(6, 26, 6, 6);
    ui->statusLabel = new QLabel("会议室未启动", statusGroup);
    ui->clientCountLabel = new QLabel("在线用户: 0", statusGroup);

    statusLayout->addWidget(ui->statusLabel);
    statusLayout->addWidget(ui->clientCountLabel);
    rightLayout->addWidget(statusGroup);

    // 用户列表
    QGroupBox *userGroup = new QGroupBox("在线用户", rightPanel);
    QVBoxLayout *userLayout = new QVBoxLayout(userGroup);
    userLayout->setContentsMargins(6, 26, 6, 6);
    ui->clientListWidget = new QListWidget(userGroup);
    userLayout->addWidget(ui->clientListWidget);
    rightLayout->addWidget(userGroup, 1);

    // 聊天区域
    QGroupBox *chatGroup = new QGroupBox("聊天", rightPanel);
    QVBoxLayout *chatLayout = new QVBoxLayout(chatGroup);
    chatLayout->setContentsMargins(6, 26, 6, 6);
    ui->chatTextEdit = new QTextEdit(chatGroup);
    ui->chatTextEdit->setReadOnly(true);
    ui->messageLineEdit = new QLineEdit(chatGroup);
    ui->messageLineEdit->setPlaceholderText("输入消息...");

    QWidget *chatButtonWidget = new QWidget(chatGroup);
    QHBoxLayout *chatButtonLayout = new QHBoxLayout(chatButtonWidget);
    chatButtonLayout->setContentsMargins(0, 0, 0, 0);

    ui->sendButton = new QPushButton("发送", chatButtonWidget);
    ui->broadcastButton = new QPushButton("广播", chatButtonWidget);
    ui->kickButton = new QPushButton("踢出", chatButtonWidget);

    chatButtonLayout->addWidget(ui->sendButton);
    chatButtonLayout->addWidget(ui->broadcastButton);
    chatButtonLayout->addWidget(ui->kickButton);

    chatLayout->addWidget(ui->chatTextEdit, 1);
    chatLayout->addWidget(ui->messageLineEdit);
    chatLayout->addWidget(chatButtonWidget);

    rightLayout->addWidget(chatGroup, 2);

    // 会议室控制
    QWidget *controlWidget = new QWidget(rightPanel);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(0, 0, 0, 0);

    ui->startMeetingButton = new QPushButton("启动会议室", controlWidget);
    ui->closeMeetingButton = new QPushButton("关闭会议室", controlWidget);

    ui->startMeetingButton->setStyleSheet(buttonStyle + "background-color: #2ecc71; color: white;");
    ui->closeMeetingButton->setStyleSheet(buttonStyle + "background-color: #e74c3c; color: white;");

    controlLayout->addWidget(ui->startMeetingButton);
    controlLayout->addWidget(ui->closeMeetingButton);

    rightLayout->addWidget(controlWidget);

    // 添加到主布局
    mainLayout->addWidget(videoArea, 2);
    mainLayout->addWidget(rightPanel, 1);

    // 初始化按钮状态
    ui->closeMeetingButton->setEnabled(false);
    ui->stopVideoButton->setEnabled(false);

    // 连接信号槽
    connect(ui->startMeetingButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_startMeetingButton_clicked);
    connect(ui->closeMeetingButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_closeMeetingButton_clicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_sendButton_clicked);
    connect(ui->broadcastButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_broadcastButton_clicked);
    connect(ui->kickButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_kickButton_clicked);
    connect(ui->startVideoButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_startVideoButton_clicked);
    connect(ui->stopVideoButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_stopVideoButton_clicked);
    connect(ui->sendImageButton, &QPushButton::clicked, this, &MeetingRoomDialog::on_sendImageButton_clicked);
}

void MeetingRoomDialog::assignVideoFrame(const QString &username, const QString &userType)
{
    // 服务器固定占用索引0，从索引1开始分配
    for (int i = 1; i < infoLabels.size(); ++i) {
        if (infoLabels[i]->text() == "空闲") {
            infoLabels[i]->setText(QString("%1 (%2)").arg(username).arg(userType));
            videoLabels[i]->setText("等待视频流...");
            userFrameMap[username] = i;
            break;
        }
    }
}

void MeetingRoomDialog::releaseVideoFrame(const QString &username)
{
    if (userFrameMap.contains(username)) {
        int frameIndex = userFrameMap[username];
        infoLabels[frameIndex]->setText("空闲");
        videoLabels[frameIndex]->setText("等待用户加入...");
        videoLabels[frameIndex]->clear();
        userFrameMap.remove(username);
    }
}

#ifdef QT_MULTIMEDIA_LIB
void MeetingRoomDialog::sendVideoFrame(const QImage &image)
{
    if (meetingClients.isEmpty()) {
        qDebug() << "没有客户端连接，不发送视频帧";
        return;
    }

    qDebug() << "准备发送视频帧，图像尺寸:" << image.size() << "客户端数量:" << meetingClients.size();

    QByteArray imgData;
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    if (image.save(&buffer, "JPEG", 50)) {
        qDebug() << "图像压缩成功，数据大小:" << imgData.size() << "bytes";
    } else {
        qDebug() << "图像压缩失败";
        return;
    }

    // 使用JSON格式发送视频帧
    QJsonObject videoMsg;
    videoMsg["type"] = "video_frame";
    videoMsg["data"] = QString(imgData.toBase64());
    videoMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");
    videoMsg["user"] = "服务器";

    broadcastMessage(videoMsg);
    qDebug() << "视频帧已广播";
}

#endif
