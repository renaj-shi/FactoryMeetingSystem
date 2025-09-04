#include "meetingdialog.h"
#include "fakevideothread.h"
#include "audioprocessor.h"
#include <QBuffer>
#include <QImage>
#include <QPixmap>
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
#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QBuffer>
#include <QByteArray>
#include <QPixmap>
#include <QImage>
MeetingDialog::MeetingDialog(const QString &username,
                             const QString &server,
                             int port,  // 改为 int
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
    , sendImageButton(nullptr)
    , audioProcessor(new AudioProcessor(this))
    , audioButton(nullptr)
{
    setWindowTitle("远程会议室 - " + username);
    resize(800, 600);

    setupUI();

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
    sendImageButton->setEnabled(false);

    addMessageToChat("系统", "正在连接到会议服务器...");

    // 连接到服务器
    tcpSocket->connectToHost("localhost", 12347);

    // 初始化音频处理器
    if (!audioProcessor->initialize()) {
        QMessageBox::warning(this, "音频错误", "无法初始化音频设备");
    }
}

MeetingDialog::~MeetingDialog()
{
    if (isInMeeting) {
        sendLeaveMeeting();
    }
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
}

void MeetingDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel *titleLabel = new QLabel("远程会议室", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 聊天区域
    chatTextEdit = new QTextEdit(this);
    chatTextEdit->setReadOnly(true);
    chatTextEdit->setStyleSheet("background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 4px;");
    mainLayout->addWidget(chatTextEdit, 1);

    // 消息输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(this);
    messageEdit->setPlaceholderText("输入消息...");
    messageEdit->setStyleSheet("padding: 8px; border: 1px solid #e2e8f0; border-radius: 4px;");

    // 添加发送图片按钮
    sendImageButton = new QPushButton("发送图片", this);
    sendImageButton->setStyleSheet(
        "QPushButton { background-color: #8b5cf6; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #7c3aed; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );


    sendButton = new QPushButton("发送", this);
    sendButton->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    inputLayout->addWidget(messageEdit, 1);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);
    inputLayout->addWidget(sendImageButton);

    // 会议控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    joinMeetingButton = new QPushButton("加入会议", this);
    joinMeetingButton->setStyleSheet(
        "QPushButton { background-color: #10b981; color: white; padding: 10px 20px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #059669; }"
        );

    leaveMeetingButton = new QPushButton("离开会议", this);
    leaveMeetingButton->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; padding: 10px 20px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );

    controlLayout->addWidget(joinMeetingButton);
    controlLayout->addWidget(leaveMeetingButton);
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);

    // 右侧：视频区域
    setupVideoUI();

    // 添加到主布局
    mainLayout->addWidget(this, 2); // 聊天区域占2份
    mainLayout->addWidget(videoContainerWidget, 1); // 视频区域占1份


    // 添加音频控制按钮
    audioButton = new QPushButton("开启语音", this);
    audioButton->setStyleSheet(
        "QPushButton { background-color: #f59e0b; color: white; padding: 8px 16px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #d97706; }"
        "QPushButton:disabled { background-color: #cbd5e1; }"
        );
    inputLayout->addWidget(audioButton);




    // 连接按钮信号
    connect(joinMeetingButton, &QPushButton::clicked, this, &MeetingDialog::onJoinMeetingClicked);
    connect(leaveMeetingButton, &QPushButton::clicked, this, &MeetingDialog::onLeaveMeetingClicked);
    connect(sendButton, &QPushButton::clicked, this, &MeetingDialog::onSendButtonClicked);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MeetingDialog::onSendButtonClicked);
    connect(sendImageButton, &QPushButton::clicked, this, &MeetingDialog::onSendImageClicked); // 连接图片按钮信号
    connect(audioButton, &QPushButton::clicked, this, &MeetingDialog::onAudioButtonClicked); // 连接音频按钮信号
    connect(audioProcessor, &AudioProcessor::audioDataReady, this, &MeetingDialog::onAudioDataReady);// 连接音频处理器信号

}

// 以下是原有的功能逻辑实现
void MeetingDialog::onSocketConnected()
{
    addMessageToChat("系统", "已连接到会议服务器");
}

void MeetingDialog::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QMessageBox::warning(this, "连接错误",
                         QString("无法连接到会议服务器: %1").arg(tcpSocket->errorString()));
    leaveMeetingButton->setEnabled(false);
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
                if (type == "video_frame")
                    processVideoFrame(json);
                else
                    handleServerMessage(json);
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

void MeetingDialog::processVideoFrame(const QJsonObject &json)
{
    QString type = json["type"].toString();
    if (type == "video_frame") {
        QString base64Data = json["data"].toString();
        QByteArray imgData = QByteArray::fromBase64(base64Data.toUtf8());
        QImage image;
        if (image.loadFromData(imgData, "JPEG"))
            onVideoFrameReceived(image);
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
        sendImageButton->setEnabled(true);  // 启用图片发送按钮
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

}

void MeetingDialog::onSocketDisconnected()
{
    QMessageBox::warning(this, "连接断开", "与会议服务器的连接已断开");
    isInMeeting = false;
    leaveMeetingButton->setEnabled(false);
    joinMeetingButton->setEnabled(false);
    sendButton->setEnabled(false);
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

void MeetingDialog::setupVideoUI()
{
    // 创建视频容器widget
    videoContainerWidget = new QWidget(this);
    videoContainerWidget->setMinimumWidth(320);
    QVBoxLayout *videoLayout = new QVBoxLayout(videoContainerWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(12);

    // 视频标题
    QLabel *videoTitle = new QLabel("视频通话", videoContainerWidget);
    videoTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1e293b; padding: 5px 0;");
    videoTitle->setAlignment(Qt::AlignCenter);
    videoLayout->addWidget(videoTitle);

    // 视频显示区域
    QFrame *videoFrame = new QFrame(videoContainerWidget);
    videoFrame->setStyleSheet(R"(
        QFrame {
            background-color: #000;
            border-radius: 8px;
            border: 1px solid #e2e8f0;
        }
    )");
    QVBoxLayout *frameLayout = new QVBoxLayout(videoFrame);
    frameLayout->setContentsMargins(12, 12, 12, 12);
    frameLayout->setSpacing(12);

    // 本地视频
    QLabel *localTitle = new QLabel("本地视频", videoFrame);
    localTitle->setStyleSheet("color: #ffffff; font-size: 12px;");
    localTitle->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(localTitle);

    localVideoLabel = new QLabel(videoFrame);
    localVideoLabel->setAlignment(Qt::AlignCenter);
    localVideoLabel->setStyleSheet(R"(
        QLabel {
            background-color: #2d3748;
            border: 2px solid #4a5568;
            border-radius: 6px;
            color: #a0aec0;
            min-height: 120px;
        }
    )");
    localVideoLabel->setMinimumSize(280, 160);
    frameLayout->addWidget(localVideoLabel);

    // 远程视频
    QLabel *remoteTitle = new QLabel("远程视频", videoFrame);
    remoteTitle->setStyleSheet("color: #ffffff; font-size: 12px;");
    remoteTitle->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(remoteTitle);

    remoteVideoLabel = new QLabel(videoFrame);
    remoteVideoLabel->setAlignment(Qt::AlignCenter);
    remoteVideoLabel->setStyleSheet(R"(
        QLabel {
            background-color: #2d3748;
            border: 2px solid #4a5568;
            border-radius: 6px;
            color: #a0aec0;
            min-height: 120px;
        }
    )");
    remoteVideoLabel->setMinimumSize(280, 160);
    frameLayout->addWidget(remoteVideoLabel);

    videoLayout->addWidget(videoFrame, 1);

    // 视频控制按钮
    QHBoxLayout *videoButtonLayout = new QHBoxLayout();
    videoButtonLayout->setSpacing(8);

    startVideoButton = new QPushButton("开始视频", videoContainerWidget);
    startVideoButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
            color: white;
            padding: 10px 16px;
            border-radius: 6px;
            border: none;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
        QPushButton:disabled {
            background-color: #cbd5e1;
        }
    )");

    stopVideoButton = new QPushButton("停止视频", videoContainerWidget);
    stopVideoButton->setStyleSheet(R"(
        QPushButton {
            background-color: #6b7280;
            color: white;
            padding: 10px 16px;
            border-radius: 6px;
            border: none;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #4b5563;
        }
        QPushButton:pressed {
            background-color: #374151;
        }
        QPushButton:disabled {
            background-color: #cbd5e1;
        }
    )");
    stopVideoButton->setEnabled(false);

    videoButtonLayout->addWidget(startVideoButton);
    videoButtonLayout->addWidget(stopVideoButton);
    videoButtonLayout->addStretch();
    videoLayout->addLayout(videoButtonLayout);

    // 连接视频按钮信号
    connect(startVideoButton, &QPushButton::clicked, this, &MeetingDialog::startVideo);
    connect(stopVideoButton,  &QPushButton::clicked, this, &MeetingDialog::stopVideo);

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
