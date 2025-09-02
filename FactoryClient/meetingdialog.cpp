#include "meetingdialog.h"
#include "audioprocessor.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QDebug>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QBuffer>
#include <QByteArray>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#ifdef QT_MULTIMEDIA_LIB
#include <QCameraInfo>
#endif

MeetingDialog::MeetingDialog(const QString &username,
                             const QString &server,
                             int port,
                             const QString &meetingId,
                             QWidget *parent)
    : QDialog(parent)
    , username(username)
    , isInMeeting(false)
    , tcpSocket(nullptr)
    , chatTextEdit(nullptr)
    , messageEdit(nullptr)
    , sendButton(nullptr)
    , joinMeetingButton(nullptr)
    , leaveMeetingButton(nullptr)
    , localVideoLabel(nullptr)
    , remoteVideoLabel(nullptr)
    , startVideoButton(nullptr)
    , stopVideoButton(nullptr)
    , videoContainer(nullptr)
    , fakeVideoThread(nullptr)
    , audioProcessor(new AudioProcessor(this))
    , audioButton(nullptr)
{
    setWindowTitle("远程会议室 - " + username);
    //resize(1000, 700);

    setupUI();
    //设置视频【new】
    setupVideoUI();
    // 创建TCP socket
    tcpSocket = new QTcpSocket(this);

    // 连接信号槽
    connect(tcpSocket, &QTcpSocket::connected, this, &MeetingDialog::onSocketConnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MeetingDialog::onSocketReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MeetingDialog::onSocketDisconnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &MeetingDialog::onSocketError);

    // 初始状态
    leaveMeetingButton->setEnabled(false);
    joinMeetingButton->setEnabled(true);
    sendButton->setEnabled(false);
    startVideoButton->setEnabled(false);
    stopVideoButton->setEnabled(false);

    // 初始化音频处理器
    if (!audioProcessor->initialize()) {
        QMessageBox::warning(this, "音频错误", "无法初始化音频设备");
    }
    addMessageToChat("系统", "正在连接到会议服务器...");

    // 连接到服务器
    tcpSocket->connectToHost(server, port);
}

MeetingDialog::~MeetingDialog()
{
    if (isInMeeting) {
        sendLeaveMeeting();
    }
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
#ifdef QT_MULTIMEDIA_LIB
    stopVideo();
#endif
}

void MeetingDialog::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this); // 改为水平布局

    // 左侧：聊天区域
    QWidget *chatWidget = new QWidget(this);
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);
    chatLayout->setContentsMargins(0, 0, 0, 0);

    // 标题
    QLabel *titleLabel = new QLabel("远程会议室 - " + username, chatWidget);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1e293b; margin-bottom: 10px;");
    chatLayout->addWidget(titleLabel);

    // 聊天区域
    chatTextEdit = new QTextEdit(chatWidget);
    chatTextEdit->setReadOnly(true);
    chatTextEdit->setStyleSheet("background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 4px;");
    chatLayout->addWidget(chatTextEdit, 1);

    // 消息输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(chatWidget);
    messageEdit->setPlaceholderText("输入消息...");
    messageEdit->setStyleSheet("padding: 8px; border: 1px solid #e2e8f0; border-radius: 4px;");

    sendButton = new QPushButton("发送", chatWidget);
    sendButton->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

        // 添加发送图片按钮
    sendImageButton = new QPushButton("发送图片", this);
    sendImageButton->setStyleSheet(
        "QPushButton { background-color: #8b5cf6; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #7c3aed; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    inputLayout->addWidget(messageEdit, 1);
    inputLayout->addWidget(sendButton);
    chatLayout->addLayout(inputLayout);
    inputLayout->addWidget(sendImageButton);

    // 添加音频控制按钮
    audioButton = new QPushButton("开启语音", this);
    audioButton->setStyleSheet(
        "QPushButton { background-color: #f59e0b; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #d97706; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );
    inputLayout->addWidget(audioButton);

    // 会议控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    joinMeetingButton = new QPushButton("加入会议", chatWidget);
    joinMeetingButton->setStyleSheet(
        "QPushButton { background-color: #10b981; color: white; padding: 10px 20px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #059669; }"
        );

    leaveMeetingButton = new QPushButton("离开会议", chatWidget);
    leaveMeetingButton->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; padding: 10px 20px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    controlLayout->addWidget(joinMeetingButton);
    controlLayout->addWidget(leaveMeetingButton);
    controlLayout->addStretch();
    chatLayout->addLayout(controlLayout);

    mainLayout->addWidget(chatWidget, 2); // 聊天区域占2份

    // 连接按钮信号
    connect(joinMeetingButton, &QPushButton::clicked, this, &MeetingDialog::onJoinMeetingClicked);
    connect(leaveMeetingButton, &QPushButton::clicked, this, &MeetingDialog::onLeaveMeetingClicked);
    connect(sendButton, &QPushButton::clicked, this, &MeetingDialog::onSendButtonClicked);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MeetingDialog::onSendButtonClicked);
    connect(sendImageButton, &QPushButton::clicked, this, &MeetingDialog::onSendImageClicked); 
    // 连接音频按钮信号
    connect(audioButton, &QPushButton::clicked, this, &MeetingDialog::onAudioButtonClicked);
    // 连接音频处理器信号
    connect(audioProcessor, &AudioProcessor::audioDataReady, this, &MeetingDialog::onAudioDataReady);
    
}
void MeetingDialog::setupVideoUI()
{
    // 右侧：视频区域
    QWidget *videoWidget = new QWidget(this);
    QVBoxLayout *videoLayout = new QVBoxLayout(videoWidget);
    videoLayout->setContentsMargins(10, 0, 0, 0);

    // 视频标题
    QLabel *videoTitle = new QLabel("视频通话", videoWidget);
    videoTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1e293b; margin-bottom: 10px;");
    videoLayout->addWidget(videoTitle);

    // 视频容器
    videoContainer = new QWidget(videoWidget);
    videoContainer->setStyleSheet("background-color: #000; border-radius: 4px;");
    QVBoxLayout *containerLayout = new QVBoxLayout(videoContainer);

    // 本地视频标签
    localVideoLabel = new QLabel("本地视频", videoContainer);
    localVideoLabel->setAlignment(Qt::AlignCenter);
    localVideoLabel->setStyleSheet("background-color: #2d3748; color: white; border: 1px solid #4a5568; min-height: 150px;");
    localVideoLabel->setMinimumSize(320, 180);
    containerLayout->addWidget(localVideoLabel);

    // 远程视频标签
    remoteVideoLabel = new QLabel("远程视频", videoContainer);
    remoteVideoLabel->setAlignment(Qt::AlignCenter);
    remoteVideoLabel->setStyleSheet("background-color: #2d3748; color: white; border: 1px solid #4a5568; min-height: 150px; margin-top: 10px;");
    remoteVideoLabel->setMinimumSize(320, 180);
    containerLayout->addWidget(remoteVideoLabel);

    videoLayout->addWidget(videoContainer);

    // 视频控制按钮
    QHBoxLayout *videoButtonLayout = new QHBoxLayout();
    startVideoButton = new QPushButton("开始视频", videoWidget);
    startVideoButton->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    stopVideoButton = new QPushButton("停止视频", videoWidget);
    stopVideoButton->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    videoButtonLayout->addWidget(startVideoButton);
    videoButtonLayout->addWidget(stopVideoButton);
    videoButtonLayout->addStretch();
    videoLayout->addLayout(videoButtonLayout);

    // 添加到主布局
    QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(videoWidget, 1); // 视频区域占1份
    }

    // 连接视频按钮信号
    connect(startVideoButton, &QPushButton::clicked, this, &MeetingDialog::onStartVideoClicked);
    connect(stopVideoButton, &QPushButton::clicked, this, &MeetingDialog::onStopVideoClicked);
}

// 以下是原有的功能逻辑实现
void MeetingDialog::onSocketConnected()
{
    addMessageToChat("系统", "已连接到会议服务器");
    startVideoButton->setEnabled(true);
}

void MeetingDialog::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QMessageBox::warning(this, "连接错误",
                         QString("无法连接到会议服务器: %1").arg(tcpSocket->errorString()));
    leaveMeetingButton->setEnabled(false);
    startVideoButton->setEnabled(false);
    stopVideoButton->setEnabled(false);
}

void MeetingDialog::onJoinMeetingClicked()
{
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "连接错误", "与会议服务器的连接已断开");
        return;
    }
    sendJoinMeeting();
}

void MeetingDialog::onLeaveMeetingClicked()
{
    sendLeaveMeeting();
#ifdef QT_MULTIMEDIA_LIB
    stopVideo();
#endif
}

void MeetingDialog::onSendButtonClicked()
{
    QString message = messageEdit->text().trimmed();

    if (message.isEmpty() || !isInMeeting) {
        return;
    }

    sendChatMessage(message);
    messageEdit->clear();
}

void MeetingDialog::onStartVideoClicked()
{
#ifdef QT_MULTIMEDIA_LIB
    startVideo();
#else
    addMessageToChat("系统", "视频功能不可用（未启用多媒体支持）");
#endif
}
// meetingdialog.cpp
// 请直接覆盖原来的 startVideo / stopVideo
// 其余代码保持不变

void MeetingDialog::startVideo()
{
    // 如果已启动，则忽略
    if (fakeVideoThread)
        return;

    //指定本地视频文件地址
    QString localVideoPath = ":/videos/my_video.mp4";

    // 生成一张纯色测试图（640×480 绿色）
    QFile videoFile(localVideoPath);
    if (videoFile.exists()) {
        // 使用本地视频文件作为伪装源
        fakeVideoThread = new FakeVideoThread(this);
        fakeVideoThread->setVideoFile(localVideoPath);

        connect(fakeVideoThread, &FakeVideoThread::frameReady,
                this, [this](const QImage &img){
                    // 本地预览
                    localVideoLabel->setPixmap(
                        QPixmap::fromImage(img).scaled(
                            localVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    // 发送给服务器
                    sendVideoFrame(img);
                });
        fakeVideoThread->start();

        addMessageToChat("系统", QString("正在播放本地视频: %1").arg(QFileInfo(localVideoPath).fileName()));
    } else {
        // 如果视频文件不存在，使用默认的纯色测试图
        addMessageToChat("系统", "视频文件不存在，使用默认虚拟视频");

        QImage fakeFrame(640, 480, QImage::Format_RGB32);
        fakeFrame.fill(Qt::green);

        fakeVideoThread = new FakeVideoThread(fakeFrame, this);
        connect(fakeVideoThread, &FakeVideoThread::frameReady,
                this, [this](const QImage &img){
                    localVideoLabel->setPixmap(
                        QPixmap::fromImage(img).scaled(
                            localVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    sendVideoFrame(img);
                });
        fakeVideoThread->start();
    }

    startVideoButton->setEnabled(false);
    stopVideoButton->setEnabled(true);
    addMessageToChat("系统", "虚拟视频已启动");
}

void MeetingDialog::stopVideo()
{
    if (fakeVideoThread) {
        fakeVideoThread->stop();
        fakeVideoThread->deleteLater();
        fakeVideoThread = nullptr;
    }

    localVideoLabel->clear();
    localVideoLabel->setText("本地视频");
    remoteVideoLabel->clear();
    remoteVideoLabel->setText("远程视频");

    startVideoButton->setEnabled(isInMeeting);
    stopVideoButton->setEnabled(false);
    addMessageToChat("系统", "虚拟视频已停止");
}

void MeetingDialog::sendVideoFrame(const QImage &frame)
{
#ifdef QT_MULTIMEDIA_LIB
    if (!isInMeeting || !tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    // 发送视频帧到服务器
    QByteArray imgData;
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 50);

    // 调整图像大小以提高传输效率
    QImage scaledImage = frame.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledImage.save(&buffer, "JPEG", 50);  // 50% 质量

    QJsonObject videoMsg;
    videoMsg["type"] = "video_frame";
    videoMsg["data"] = QString(imgData.toBase64());
    videoMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    QJsonDocument doc(videoMsg);
    QByteArray data = "JSON:" + doc.toJson(QJsonDocument::Compact) + "\n";
    tcpSocket->write(data);
#endif
}

void MeetingDialog::onVideoFrameReceived(const QImage &frame)
{
    QPixmap pixmap = QPixmap::fromImage(frame);
    remoteVideoLabel->setPixmap(pixmap.scaled(remoteVideoLabel->size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
}

void MeetingDialog::processVideoFrame(const QJsonObject &json)
{
    QString type = json["type"].toString();
    if (type == "video_frame") {
        QString base64Data = json["data"].toString();
        QByteArray imgData = QByteArray::fromBase64(base64Data.toUtf8());
        QImage image;
        if (image.loadFromData(imgData, "JPEG")) {
            onVideoFrameReceived(image);
        }
    }
}

void MeetingDialog::onStopVideoClicked()
{
#ifdef QT_MULTIMEDIA_LIB
    stopVideo();
#else
    addMessageToChat("系统", "视频功能不可用");
#endif
}

void MeetingDialog::sendChatMessage(const QString &message)
{
    QJsonObject chatMsg;
    chatMsg["type"] = "chat";
    chatMsg["user"] = username;
    chatMsg["message"] = message;
    chatMsg["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    QJsonDocument doc(chatMsg);
    QByteArray data = "JSON:" + doc.toJson(QJsonDocument::Compact) + "\n";

    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(data);
        addMessageToChat("我", message);
    } else {
        QMessageBox::warning(this, "错误", "发送消息失败：连接已断开");
    }
}

void MeetingDialog::sendJoinMeeting()
{
    QJsonObject joinMsg;
    joinMsg["type"] = "join_meeting";
    joinMsg["user"] = username;
    joinMsg["user_type"] = "factory";

    QJsonDocument doc(joinMsg);
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write("JSON:" + doc.toJson(QJsonDocument::Compact) + "\n");
        addMessageToChat("系统", "正在加入会议室...");
    } else {
        QMessageBox::warning(this, "错误", "与服务器的连接已断开");
    }
    leaveMeetingButton->setEnabled(true);
    joinMeetingButton->setEnabled(false);
    sendButton->setEnabled(true);
    sendImageButton->setEnabled(true);
    audioButton->setEnabled(true); // 启用音频按钮
}

void MeetingDialog::sendLeaveMeeting()
{
    // 关闭音频
    if (isAudioEnabled) {
        audioProcessor->stopCapture();
        isAudioEnabled = false;
    }
    
    QJsonObject leaveMsg;
    leaveMsg["type"] = "leave_meeting";
    leaveMsg["user"] = username;

    QJsonDocument doc(leaveMsg);
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write("JSON:" + doc.toJson(QJsonDocument::Compact) + "\n");
    }

    isInMeeting = false;
    leaveMeetingButton->setEnabled(false);
    joinMeetingButton->setEnabled(true);
    sendButton->setEnabled(false);
    sendImageButton->setEnabled(false);
    audioButton->setEnabled(false);
    audioButton->setText("开启语音");

    addMessageToChat("系统", "已离开会议室");
}

void MeetingDialog::onSocketReadyRead()
{
    while (tcpSocket->canReadLine()) {
        QByteArray data = tcpSocket->readLine().trimmed();

        if (data.startsWith("JSON:")) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data.mid(5), &error);
            if (error.error == QJsonParseError::NoError && !doc.isNull()) {
                QJsonObject json = doc.object();
                QString type = json["type"].toString();

                if (type == "video_frame") {
                    processVideoFrame(json);
                } else {
                    handleServerMessage(json);
                }
            }
        }
        else if (data.startsWith("AUDIO:")) {
            // 处理音频数据
            if (isAudioEnabled) {
                QByteArray audioData = QByteArray::fromBase64(data.mid(6));
                audioProcessor->playAudio(audioData);
            }
        }
    }
}

void MeetingDialog::handleServerMessage(const QJsonObject &json)
{
    QString type = json["type"].toString();

    if (type == "user_joined") {
        QString user = json["user"].toString();
        QString userType = json["user_type"].toString();
        addMessageToChat("系统", QString("用户 %1 (%2) 加入了会议室").arg(user).arg(userType));
    }
    else if (type == "user_left") {
        QString user = json["user"].toString();
        QString userType = json["user_type"].toString();
        addMessageToChat("系统", QString("用户 %1 (%2) 离开了会议室").arg(user).arg(userType));
    }
    else if (type == "chat") {
        QString user = json["user"].toString();
        QString message = json["message"].toString();
        if (user != username) {
            addMessageToChat(user, message);
        }
    }
    else if (type == "system_broadcast") {
        QString message = json["message"].toString();
        addMessageToChat("系统广播", message);
    }
    else if (type == "meeting_joined") {
        isInMeeting = true;
        leaveMeetingButton->setEnabled(true);
        joinMeetingButton->setEnabled(false);
        sendButton->setEnabled(true);
        startVideoButton->setEnabled(true);
        audioButton->setEnabled(true);  // 启用音频按钮
        addMessageToChat("系统", "已成功加入会议室");
    }
    else if (type == "image_message") {
        QString user = json["user"].toString();
        QString imageData = json["image_data"].toString();
        
        QString formattedMessage = QString("<br><b>%1</b>: <img src='data:image/png;base64,%2' style='max-width:200px;max-height:200px;' /><br>")
                                   .arg(user).arg(imageData);
        chatTextEdit->insertHtml(formattedMessage);
        
        // 滚动到底部
        QScrollBar *scrollbar = chatTextEdit->verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
    }
    else if (type == "audio_message") {
        // 处理音频消息
        QString user = json["user"].toString();
        QString audioData = json["audio_data"].toString();
        
        if (user != username && isAudioEnabled) {
            QByteArray decodedData = QByteArray::fromBase64(audioData.toLatin1());
            audioProcessor->playAudio(decodedData);
        }
    }
    else if (type == "meeting_error") {
        QString errorMsg = json["message"].toString();
        addMessageToChat("系统错误", errorMsg);
        leaveMeetingButton->setEnabled(true);
    }
    else if (type == "meeting_closed") {
        QString message = json["message"].toString();
        addMessageToChat("系统", message);
        onLeaveMeetingClicked();
    }
}

void MeetingDialog::onSocketDisconnected()
{
    QMessageBox::warning(this, "连接断开", "与会议服务器的连接已断开");
    isInMeeting = false;
    leaveMeetingButton->setEnabled(false);
    joinMeetingButton->setEnabled(false);
    sendButton->setEnabled(false);
    startVideoButton->setEnabled(false);
    stopVideoButton->setEnabled(false);

#ifdef QT_MULTIMEDIA_LIB
    stopVideo();
#endif

    emit meetingClosed();
}

void MeetingDialog::addMessageToChat(const QString &sender, const QString &message)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMessage;

    if (sender == "系统" || sender == "系统广播" || sender == "系统错误") {
        formattedMessage = QString("<font color='gray'>[%1] %2: %3</font>")
        .arg(time).arg(sender).arg(message);
    } else if (sender == "我") {
        formattedMessage = QString("<font color='blue'>[%1] <b>%2</b>: %3</font>")
        .arg(time).arg(sender).arg(message);
    } else {
        formattedMessage = QString("[%1] <b>%2</b>: %3")
        .arg(time).arg(sender).arg(message);
    }

    chatTextEdit->append(formattedMessage);
    QScrollBar *scrollbar = chatTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void MeetingDialog::onSendImageClicked()
{
    if (!isInMeeting) {
        QMessageBox::warning(this, "错误", "请先加入会议室");
        return;
    }
    
    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", "", 
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    
    if (!imagePath.isEmpty()) {
        sendImageMessage(imagePath);
    }
}

void MeetingDialog::sendImageMessage(const QString &imagePath)
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

    QJsonObject json;
    json["type"] = "image_message";
    json["user"] = username;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");
    json["image_data"] = QString(base64Data);

    QJsonDocument doc(json);
    QByteArray data = "JSON:" + doc.toJson(QJsonDocument::Compact) + "\n";
    tcpSocket->write(data);

    // 在本地显示发送的图片
    QString formattedMessage = QString("<br><b>我</b>: <img src='data:image/png;base64,%1' style='max-width:200px;max-height:200px;' /><br>")
                              .arg(QString(base64Data));
    chatTextEdit->insertHtml(formattedMessage);
    
    // 滚动到底部
    QScrollBar *scrollbar = chatTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void MeetingDialog::onAudioButtonClicked()
{
    if (!isInMeeting) {
        QMessageBox::warning(this, "错误", "请先加入会议室");
        return;
    }

    if (!isAudioEnabled) {
        // 开启语音
        audioProcessor->startCapture();
        isAudioEnabled = true;
        audioButton->setText("关闭语音");
        addMessageToChat("系统", "语音通话已开启");
    } else {
        // 关闭语音
        audioProcessor->stopCapture();
        isAudioEnabled = false;
        audioButton->setText("开启语音");
        addMessageToChat("系统", "语音通话已关闭");
    }
}

void MeetingDialog::onAudioDataReady(const QByteArray &audioData)
{
    // 发送音频数据到服务器
    if (isAudioEnabled && tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = "AUDIO:" + audioData.toBase64() + "\n";
        tcpSocket->write(data);
    }
}