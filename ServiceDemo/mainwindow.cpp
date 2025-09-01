#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>

#include "devicedialog.h"
#include "ticketsdialog.h"
#include "deviceworker.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , factoryServer(new QTcpServer(this))
    , expertServer(new QTcpServer(this))
{
    ui->setupUi(this);
    setWindowTitle("服务端");

    if (!initDatabase()) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法初始化数据库: " + db.lastError().text());
        return;
    }

    if (!factoryServer->listen(QHostAddress::Any, 12345)) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法启动工厂服务器接口: " + factoryServer->errorString());
        return;
    } else {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 工厂服务器启动成功，监听端口: 12345");
    }

    if (!expertServer->listen(QHostAddress::Any, 12346)) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 无法启动专家服务器接口: " + expertServer->errorString());
        return;
    } else {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 专家服务器启动成功，监听端口: 12346");
    }

    connect(factoryServer, &QTcpServer::newConnection, this, &MainWindow::onFactoryNewConnection);
    connect(expertServer, &QTcpServer::newConnection, this, &MainWindow::onExpertNewConnection);

    ui->statusbar->showMessage("Factory server: 12345, Expert server: 12346");
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                         " - 服务器启动完成，等待客户端连接...");
}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
}

bool MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("industrial.db");

    if (!db.open()) {
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                             " - 数据库打开错误: " + db.lastError().text());
        return false;
    }

    QSqlQuery query;
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS factory_users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS expert_users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS tickets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            priority TEXT NOT NULL,
            status INTEGER DEFAULT 0,
            creator TEXT NOT NULL,
            expert_username TEXT,
            created_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            solution TEXT,
            device_params TEXT
        )
    )")) return false;

    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS device_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temperature REAL, pressure REAL, vibration REAL,
            current REAL, voltage REAL, speed INTEGER,
            status BOOLEAN,
            record_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )")) return false;

    // 新增：索引与“简化知识库”关联表/故障表
    query.exec("CREATE INDEX IF NOT EXISTS idx_device_history_time ON device_history(record_time)");

    query.exec(R"(
      CREATE TABLE IF NOT EXISTS faults (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ticket_id INTEGER,
        code TEXT, text TEXT, level TEXT,
        ts DATETIME DEFAULT CURRENT_TIMESTAMP
    ))");
    query.exec("CREATE INDEX IF NOT EXISTS idx_faults_ticket ON faults(ticket_id)");

    query.exec(R"(
      CREATE TABLE IF NOT EXISTS ticket_device_history (
        ticket_id INTEGER NOT NULL,
        history_id INTEGER NOT NULL,
        PRIMARY KEY(ticket_id, history_id)
    ))");

    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - 数据库初始化完成");
    return true;
}

void MainWindow::showTicketsDialog()
{
    TicketsDialog dlg(this);
    dlg.exec();
}

void MainWindow::onFactoryNewConnection()
{
    QTcpSocket *socket = factoryServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    clientTypeMap.insert(socket, "FACTORY");

    const QString clientInfo = QString("工厂客户端连接: %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + clientInfo);

    sendResponse(socket, "TYPE|FACTORY|ACK");
}

void MainWindow::onExpertNewConnection()
{
    QTcpSocket *socket = expertServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    clientTypeMap.insert(socket, "EXPERT");

    expertSockets.insert(socket);
    connect(socket, &QTcpSocket::disconnected, this, [=]{
        expertSockets.remove(socket);
        const QString oid = expertJoinedOrder.take(socket);
        if (!oid.isEmpty()) {
            orderExperts[oid].remove(socket);
            teleSubs[oid].remove(socket);
        }
    });

    const QString clientInfo = QString("专家客户端连接: %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " - " + clientInfo);

    sendResponse(socket, "TYPE|EXPERT|ACK");
}

void MainWindow::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        const QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (line.isEmpty()) continue;

        const QString clientType = clientTypeMap.value(socket);
        const QString clientInfo = QString("%1客户端 %2:%3")
                                       .arg(clientType)
                                       .arg(socket->peerAddress().toString())
                                       .arg(socket->peerPort());
        ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                             + " - 收到来自 " + clientInfo + " 的数据: " + line);
        processLine(socket, line);
    }
}

void MainWindow::processLine(QTcpSocket* socket, const QString& data)
{
    const QStringList parts = data.split('|');
    if (parts.isEmpty()) return;

    const QString action = parts[0];
    const QString clientType = clientTypeMap.value(socket);

    if (action == "LOGIN") {
        if (parts.size() >= 3) handleLogin(socket, parts[1], parts[2], clientType);
        return;
    }

    if (action == "REGISTER") {
        if (parts.size() >= 3) handleRegister(socket, parts[1], parts[2], clientType);
        return;
    }

    if (action == "CREATE_TICKET") {
        handleCreateTicket(socket, parts);
        return;
    }

    if (action == "ORDER") {
        const QString sub = parts.value(1);
        if (sub == "JOIN") {
            const QString oid = parts.value(2);
            
            // 检查工单是否存在
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT id FROM tickets WHERE id = :id");
            checkQuery.bindValue(":id", oid.toInt());
            if (!checkQuery.exec() || !checkQuery.next()) {
                // 工单不存在
                sendResponse(socket, QString("ORDER|JOIN|ERR|工单 %1 不存在，无法加入").arg(oid));
                return;
            }
            
            // 工单存在，继续加入逻辑
            if (clientType == "EXPERT") {
                orderExperts[oid].insert(socket);
                expertJoinedOrder[socket] = oid;
            } else if (clientType == "FACTORY") {
                orderFactories[oid].insert(socket);
                factoryJoinedOrder[socket] = oid;
            }
            activeOrderId_ = oid;
            sendResponse(socket, QString("ORDER|JOIN|OK|%1").arg(oid));
            return;
        }
        if (sub == "LEAVE") {
            const QString oid = parts.value(2);
            if (clientType == "EXPERT") {
                orderExperts[oid].remove(socket);
                teleSubs[oid].remove(socket);
                expertJoinedOrder.remove(socket);
            } else if (clientType == "FACTORY") {
                orderFactories[oid].remove(socket);
                factoryJoinedOrder.remove(socket);
            }
            if (activeOrderId_ == oid) activeOrderId_.clear();
            sendResponse(socket, QString("ORDER|LEAVE|OK|%1").arg(oid));
            return;
        }
        return;
    }

    if (action == "TELE") {
        const QString sub = parts.value(1);
        const QString oid = parts.value(2);
        if (sub == "SUB")   { teleSubs[oid].insert(socket);  return; }
        if (sub == "UNSUB") { teleSubs[oid].remove(socket);  return; }
        return;
    }

    if (action == "CHAT") {
        const QString sub = parts.value(1);
        if (sub == "SEND") {
            int p0 = data.indexOf('|');
            int p1 = data.indexOf('|', p0 + 1);
            int p2 = data.indexOf('|', p1 + 1);
            int p3 = data.indexOf('|', p2 + 1);
            if (p0 < 0 || p1 < 0 || p2 < 0 || p3 < 0) {
                sendResponse(socket, "ERROR|CHAT|BAD_FORMAT");
                return;
            }
            const QString orderId = data.mid(p1 + 1, p2 - (p1 + 1));
            const QString from    = data.mid(p2 + 1, p3 - (p2 + 1));
            const QString text    = data.mid(p3 + 1);
            QString safe = text; safe.replace('\n', ' ').replace('\r', ' ');

            ui->textEdit->append(QString("CHAT SEND: oid=%1, from=%2, text=%3")
                                     .arg(orderId, from, safe.left(80)));

            broadcastChat(orderId, from, safe);
        }
        return;
    }

    // 新增：历史工单（简化知识库）
    if (action == "TICKETS") {
        const QString sub = parts.value(1);
        if (sub == "LIST") {
            bool ok1=false, ok2=false;
            int page = parts.value(2).toInt(&ok1);
            int size = parts.value(3).toInt(&ok2);
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=50;
            handleTicketsList(socket, page, size);
            return;
        }
        if (sub == "LOGS") {
            bool okId=false, ok1=false, ok2=false;
            int tid  = parts.value(2).toInt(&okId);
            int page = parts.value(3).toInt(&ok1);
            int size = parts.value(4).toInt(&ok2);
            if (!okId || tid<=0) { sendResponse(socket,"TICKETS|LOGS|ERR|bad id"); return; }
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=100;
            handleTicketLogs(socket, tid, page, size);
            return;
        }
        if (sub == "FAULTS") {
            bool okId=false, ok1=false, ok2=false;
            int tid  = parts.value(2).toInt(&okId);
            int page = parts.value(3).toInt(&ok1);
            int size = parts.value(4).toInt(&ok2);
            if (!okId || tid<=0) { sendResponse(socket,"TICKETS|FAULTS|ERR|bad id"); return; }
            if (!ok1 || page<=0) page=1;
            if (!ok2 || size<=0 || size>200) size=100;
            handleFaultsByTicket(socket, tid, page, size);
            return;
        }
        return;
    }

    sendResponse(socket, "ERROR|Unknown action");
}

void MainWindow::handleLogin(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType)
{
    QString tableName;
    if (clientType == "FACTORY")       tableName = "factory_users";
    else if (clientType == "EXPERT")   tableName = "expert_users";
    else { sendResponse(socket, "LOGIN|FAIL|Invalid client type"); return; }

    QSqlQuery query;
    query.prepare("SELECT password FROM " + tableName + " WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec()) { sendResponse(socket, "LOGIN|FAIL|DB error"); return; }

    if (query.next()) {
        if (query.value(0).toString() == password) {
            sendResponse(socket, "LOGIN|SUCCESS");
            m_socketToUsername[socket] = username;
            if (clientType == "FACTORY") {
                m_factorySockets[username] = socket;
                ui->textEdit->append("工厂客户端 " + username + " 已添加到 m_factorySockets");
            }
        } else sendResponse(socket, "LOGIN|FAIL|Invalid password");
    } else sendResponse(socket, "LOGIN|FAIL|User not found");
}

void MainWindow::handleRegister(QTcpSocket *socket, const QString &username, const QString &password, const QString &clientType)
{
    QString tableName;
    if (clientType == "FACTORY")       tableName = "factory_users";
    else if (clientType == "EXPERT")   tableName = "expert_users";
    else { sendResponse(socket, "REGISTER|FAIL|Invalid client type"); return; }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM " + tableName + " WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec()) { sendResponse(socket, "REGISTER|FAIL|DB error"); return; }
    if (checkQuery.next()) { sendResponse(socket, "REGISTER|FAIL|Username already exists"); return; }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO " + tableName + " (username, password) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    if (insertQuery.exec()) sendResponse(socket, "REGISTER|SUCCESS");
    else sendResponse(socket, "REGISTER|FAIL|DB error");
}

void MainWindow::handleCreateTicket(QTcpSocket *socket, const QStringList &parts)
{
    if (clientTypeMap.value(socket) != "FACTORY" || parts.size() < 4) {
        sendResponse(socket, "ERROR|Invalid CREATE_TICKET command format");
        return;
    }
    const QString creator = m_socketToUsername.value(socket);
    if (creator.isEmpty()) { sendResponse(socket, "ERROR|Not logged in"); return; }

    const QString title = parts.value(1);
    const QString description = parts.value(2);
    const QString priority = parts.value(3);

    QSqlQuery query;
    query.prepare("INSERT INTO tickets (title, description, priority, status, creator) "
                  "VALUES (:title, :desc, :priority, :status, :creator)");
    query.bindValue(":title", title);
    query.bindValue(":desc", description);
    query.bindValue(":priority", priority);
    query.bindValue(":status", PENDING);
    query.bindValue(":creator", creator);
    if (!query.exec()) {
        sendResponse(socket, "ERROR|Database error: " + query.lastError().text());
        return;
    }
    const int ticketId = query.lastInsertId().toInt();
    activeOrderId_ = QString::number(ticketId);

    orderFactories[activeOrderId_].insert(socket);
    factoryJoinedOrder[socket] = activeOrderId_;

    sendResponse(socket, "CREATE_TICKET|SUCCESS|" + QString::number(ticketId));
    const QString invite = QString("ORDER|INVITE|%1|%2").arg(activeOrderId_, title);
    broadcastToExperts(invite);
}

void MainWindow::sendResponse(QTcpSocket *socket, const QString &response)
{
    if (socket && socket->isOpen()) {
        socket->write(response.toUtf8());
        socket->write("\n");
        socket->flush();
    }
}

void MainWindow::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    const QString username  = m_socketToUsername.value(socket);
    const QString clientType= clientTypeMap.value(socket);

    if (clientType == "FACTORY" && !username.isEmpty()) {
        m_factorySockets.remove(username);
        ui->textEdit->append("工厂客户端 " + username + " 已从 m_factorySockets 中移除");
    }

    const QString eoid = expertJoinedOrder.take(socket);
    if (!eoid.isEmpty()) {
        orderExperts[eoid].remove(socket);
        teleSubs[eoid].remove(socket);
    }
    const QString foid = factoryJoinedOrder.take(socket);
    if (!foid.isEmpty()) {
        orderFactories[foid].remove(socket);
    }

    m_socketToUsername.remove(socket);
    clientTypeMap.remove(socket);
    ui->textEdit->append("一个客户端已断开连接。");
}

void MainWindow::on_pushButton1_clicked()
{
    if (ui->pushButton3->text() == "启动设备") {
        QMessageBox::information(this, "提示", "请先点击“启动设备”按钮！");
        return;
    }

    auto *dlg = new DeviceDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);

    // connect(dlg, &DeviceDialog::paramsUpdated, this, [this](const QJsonObject& j){
    //     if (!activeOrderId_.isEmpty()) broadcastTelemetry(activeOrderId_, j);
    // });
    connect(dlg, &DeviceDialog::workOrderCreated, this, [this](int id, const QString& title){
        activeOrderId_ = QString::number(id);
        const QString invite = QString("ORDER|INVITE|%1|%2").arg(activeOrderId_, title);
        broadcastToExperts(invite);
    });

    dlg->show();
}

void MainWindow::on_pushButton2_clicked()
{
    TicketsDialog *dlg = new TicketsDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->show();
}


void MainWindow::on_pushButton3_clicked()
{
    static bool running = false;

    if (!running) {
        m_deviceWorker = new DeviceWorker;
        m_deviceThread = new QThread(this);
        m_deviceWorker->moveToThread(m_deviceThread);
        connect(m_deviceThread, &QThread::started, m_deviceWorker, &DeviceWorker::start);
        connect(m_deviceWorker, &DeviceWorker::finished, m_deviceThread, &QThread::quit, Qt::DirectConnection);
        connect(m_deviceWorker, &DeviceWorker::finished, m_deviceWorker, &QObject::deleteLater);
        connect(m_deviceThread, &QThread::finished, m_deviceThread, &QObject::deleteLater);

        connect(m_deviceWorker, &DeviceWorker::newData, this, [this](const DeviceParams &p){
            ui->statusLineEdit->setText(p.status ? "正常" : "风险");
            ui->statusLineEdit->setStyleSheet(p.status ? "color: green;" : "color: red;");

            broadcastDeviceParams(p);

            if (!activeOrderId_.isEmpty()) {
                // 实时推送（专家订阅）
                QJsonObject obj;
                obj["ts"]         = p.lastUpdate.toMSecsSinceEpoch();
                obj["temperature"]= p.temperature;
                obj["pressure"]   = p.pressure;
                obj["vibration"]  = p.vibration;
                obj["current"]    = p.current;
                obj["voltage"]    = p.voltage;
                obj["speed"]      = p.speed;
                obj["status"]     = p.status ? 1 : 0;
                QJsonArray logs;
                logs.append(QString("T=%1,P=%2,V=%3").arg(p.temperature,0,'f',1).arg(p.pressure,0,'f',1).arg(p.vibration,0,'f',2));
                obj["logs"] = logs;
                broadcastTelemetry(activeOrderId_, obj);

                // 新增：落库 + 关联 + 故障
                QSqlQuery ins;
                ins.prepare("INSERT INTO device_history (temperature,pressure,vibration,current,voltage,speed,status,record_time) "
                            "VALUES (:t,:p,:v,:c,:volt,:spd,:st,:rt)");
                ins.bindValue(":t",   p.temperature);
                ins.bindValue(":p",   p.pressure);
                ins.bindValue(":v",   p.vibration);
                ins.bindValue(":c",   p.current);
                ins.bindValue(":volt",p.voltage);
                ins.bindValue(":spd", p.speed);
                ins.bindValue(":st",  p.status ? 1 : 0);
                ins.bindValue(":rt",  p.lastUpdate);
                if (!ins.exec()) {
                    ui->textEdit->append("写 device_history 失败: " + ins.lastError().text());
                } else {
                    const int hid = ins.lastInsertId().toInt();
                    QSqlQuery link;
                    link.prepare("INSERT OR IGNORE INTO ticket_device_history (ticket_id, history_id) VALUES (:tid,:hid)");
                    link.bindValue(":tid", activeOrderId_.toInt());
                    link.bindValue(":hid", hid);
                    if (!link.exec()) {
                        ui->textEdit->append("写 ticket_device_history 失败: " + link.lastError().text());
                    }
                }

                auto addFault = [&](const QString& code, const QString& text, const QString& level){
                    QSqlQuery f;
                    f.prepare("INSERT INTO faults (ticket_id, code, text, level, ts) VALUES (:tid,:c,:t,:l,:ts)");
                    f.bindValue(":tid", activeOrderId_.toInt());
                    f.bindValue(":c", code);
                    f.bindValue(":t", text);
                    f.bindValue(":l", level);
                    f.bindValue(":ts", p.lastUpdate);
                    if (!f.exec()) ui->textEdit->append("写 faults 失败: " + f.lastError().text());
                };
                if (p.temperature > 70.0) addFault("F_TEMP_HIGH","温度过高","中");
                if (p.pressure    > 130.0) addFault("F_PRESS_HIGH","压力过高","中");
                if (p.vibration   > 4.0)   addFault("F_VIB_HIGH", "振动过大","中");
            }
        });

        m_deviceThread->start();
        ui->pushButton3->setText("停止设备");
        running = true;
    } else {
        if (m_deviceWorker) QMetaObject::invokeMethod(m_deviceWorker, "stop", Qt::QueuedConnection);
        ui->pushButton3->setText("启动设备");
        running = false;
    }
}

void MainWindow::on_meetingButton_clicked()
{
    if (!meetingRoom) {
        meetingRoom = new MeetingRoomDialog(this);
        connect(meetingRoom, &MeetingRoomDialog::meetingClosed, this, [this]() {
            meetingRoom->deleteLater();
            meetingRoom = nullptr;
        });
    }
    meetingRoom->show();
}

void MainWindow::broadcastDeviceParams(const DeviceParams &params)
{
    const QString orderId = "ORDER_001";
    const qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    const QString jsonData = QString("{\"ts\":%1,\"temperature\":%2,\"pressure\":%3,"
                                     "\"vibration\":%4,\"current\":%5,\"voltage\":%6,"
                                     "\"speed\":%7,\"status\":%8}")
                                 .arg(timestamp)
                                 .arg(params.temperature, 0, 'f', 1)
                                 .arg(params.pressure,    0, 'f', 1)
                                 .arg(params.vibration,   0, 'f', 1)
                                 .arg(params.current,     0, 'f', 1)
                                 .arg(params.voltage,     0, 'f', 1)
                                 .arg(params.speed)
                                 .arg(params.status ? 1 : 0);

    const QString message = QString("TELE|DATA|%1|%2").arg(orderId, jsonData);
    for (QTcpSocket* s : m_factorySockets.values()) {
        if (s && s->isOpen()) sendResponse(s, message);
    }
}

void MainWindow::sendToExpert(QTcpSocket* s, const QString& line)
{
    if (s && s->isOpen()) {
        s->write(line.toUtf8());
        s->write("\n");
        s->flush();
    }
}

void MainWindow::broadcastToExperts(const QString& line)
{
    ui->textEdit->append("向所有在线专家广播: " + line);
    for (auto* s : expertSockets) sendToExpert(s, line);
}

void MainWindow::broadcastTelemetry(const QString& orderId, const QJsonObject& obj)
{
    if (!teleSubs.contains(orderId)) return;
    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    const QString line = QString("TELE|DATA|%1|%2").arg(orderId, json);
    for (auto* s : teleSubs[orderId]) sendToExpert(s, line);
}

void MainWindow::broadcastChat(const QString& orderId, const QString& from, const QString& text)
{
    const qint64 ts = QDateTime::currentMSecsSinceEpoch();
    const QString msg = QString("CHAT|MSG|%1|%2|%3|%4")
                            .arg(orderId, from, QString::number(ts), text);

    ui->textEdit->append(QString("向工单 %1 广播聊天：%2").arg(orderId, text.left(80)));

    for (QTcpSocket* s : orderExperts[orderId])   sendToExpert(s, msg);
    for (QTcpSocket* s : orderFactories[orderId]) sendToExpert(s, msg);
}

// 新增：历史工单查询实现
void MainWindow::handleTicketsList(QTcpSocket* socket, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;

    QSqlQuery q;
    QString sql = QString(
                      "SELECT id, title, description, priority, status, creator, "
                      "       IFNULL(expert_username,'') AS expert_username, created_time "
                      "FROM tickets ORDER BY created_time DESC LIMIT %1 OFFSET %2")
                      .arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|LIST|ERR|" + q.lastError().text()); return; }

    QSqlQuery qCountLog, qCountFault;
    qCountLog.prepare("SELECT COUNT(*) FROM ticket_device_history WHERE ticket_id=:tid");
    qCountFault.prepare("SELECT COUNT(*) FROM faults WHERE ticket_id=:tid");

    QJsonArray arr;
    while (q.next()) {
        const int tid = q.value("id").toInt();

        qCountLog.bindValue(":tid", tid); qCountLog.exec(); qCountLog.next();
        const int logsCount = qCountLog.value(0).toInt();

        qCountFault.bindValue(":tid", tid); qCountFault.exec(); qCountFault.next();
        const int faultsCount = qCountFault.value(0).toInt();

        QJsonObject o;
        o["id"]             = tid;
        o["title"]          = q.value("title").toString();
        o["description"]    = q.value("description").toString();
        o["priority"]       = q.value("priority").toString();
        o["status"]         = q.value("status").toInt();
        o["creator"]        = q.value("creator").toString();
        o["expert_username"]= q.value("expert_username").toString();
        o["created_time"]   = q.value("created_time").toString();
        o["logs"]           = logsCount;
        o["faults"]         = faultsCount;
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|LIST|OK|" + payload);
}

void MainWindow::handleTicketLogs(QTcpSocket* socket, int ticketId, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;
    QSqlQuery q;
    QString sql = QString(
                      "SELECT dh.id, dh.temperature, dh.pressure, dh.vibration, dh.current, dh.voltage, dh.speed, dh.status, dh.record_time "
                      "FROM ticket_device_history tdh "
                      "JOIN device_history dh ON dh.id = tdh.history_id "
                      "WHERE tdh.ticket_id=%1 "
                      "ORDER BY dh.record_time DESC LIMIT %2 OFFSET %3")
                      .arg(ticketId).arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|LOGS|ERR|" + q.lastError().text()); return; }

    QJsonArray arr;
    while (q.next()) {
        QJsonObject o;
        o["id"]          = q.value("id").toInt();
        o["temperature"] = q.value("temperature").toDouble();
        o["pressure"]    = q.value("pressure").toDouble();
        o["vibration"]   = q.value("vibration").toDouble();
        o["current"]     = q.value("current").toDouble();
        o["voltage"]     = q.value("voltage").toDouble();
        o["speed"]       = q.value("speed").toInt();
        o["status"]      = q.value("status").toBool() ? 1 : 0;
        o["record_time"] = q.value("record_time").toString();
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|LOGS|OK|" + payload);
}

void MainWindow::handleFaultsByTicket(QTcpSocket* socket, int ticketId, int page, int pageSize)
{
    const int offset = (page - 1) * pageSize;
    QSqlQuery q;
    QString sql = QString(
                      "SELECT id, code, text, level, ts FROM faults "
                      "WHERE ticket_id=%1 ORDER BY ts DESC LIMIT %2 OFFSET %3")
                      .arg(ticketId).arg(pageSize).arg(offset);
    if (!q.exec(sql)) { sendResponse(socket, "TICKETS|FAULTS|ERR|" + q.lastError().text()); return; }

    QJsonArray arr;
    while (q.next()) {
        QJsonObject o;
        o["id"]   = q.value("id").toInt();
        o["code"] = q.value("code").toString();
        o["text"] = q.value("text").toString();
        o["level"]= q.value("level").toString();
        o["ts"]   = q.value("ts").toString();
        arr.append(o);
    }
    const QString payload = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    sendResponse(socket, "TICKETS|FAULTS|OK|" + payload);
}
