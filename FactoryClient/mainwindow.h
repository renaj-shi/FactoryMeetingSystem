#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include "registerdialog.h"

// 前向声明
class MainInterfaceDialog;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_registerButton_clicked();
    void on_loginButton_clicked();
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void handleRegisterResult(bool success, const QString &message);
    void showRegisterDialog();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    bool isConnected;
    RegisterDialog *registerDialog;

    // 【增】增加当前登录用户名字段
    QString m_username;

    void setupConnection();
};

#endif // MAINWINDOW_H