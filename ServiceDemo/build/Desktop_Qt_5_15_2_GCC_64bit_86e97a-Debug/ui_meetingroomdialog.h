/********************************************************************************
** Form generated from reading UI file 'meetingroomdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEETINGROOMDIALOG_H
#define UI_MEETINGROOMDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_MeetingRoomDialog
{
public:
    QLabel *statusLabel;
    QLabel *clientCountLabel;
    QPushButton *startMeetingButton;
    QPushButton *closeMeetingButton;
    QTextEdit *chatTextEdit;
    QLineEdit *messageLineEdit;
    QListWidget *clientListWidget;
    QPushButton *sendButton;
    QPushButton *broadcastButton;
    QPushButton *kickButton;

    void setupUi(QDialog *MeetingRoomDialog)
    {
        if (MeetingRoomDialog->objectName().isEmpty())
            MeetingRoomDialog->setObjectName(QString::fromUtf8("MeetingRoomDialog"));
        MeetingRoomDialog->resize(888, 948);
        statusLabel = new QLabel(MeetingRoomDialog);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
        statusLabel->setGeometry(QRect(10, 910, 251, 41));
        clientCountLabel = new QLabel(MeetingRoomDialog);
        clientCountLabel->setObjectName(QString::fromUtf8("clientCountLabel"));
        clientCountLabel->setGeometry(QRect(10, 10, 251, 41));
        startMeetingButton = new QPushButton(MeetingRoomDialog);
        startMeetingButton->setObjectName(QString::fromUtf8("startMeetingButton"));
        startMeetingButton->setGeometry(QRect(80, 80, 171, 51));
        closeMeetingButton = new QPushButton(MeetingRoomDialog);
        closeMeetingButton->setObjectName(QString::fromUtf8("closeMeetingButton"));
        closeMeetingButton->setGeometry(QRect(80, 160, 171, 51));
        chatTextEdit = new QTextEdit(MeetingRoomDialog);
        chatTextEdit->setObjectName(QString::fromUtf8("chatTextEdit"));
        chatTextEdit->setGeometry(QRect(70, 380, 731, 411));
        messageLineEdit = new QLineEdit(MeetingRoomDialog);
        messageLineEdit->setObjectName(QString::fromUtf8("messageLineEdit"));
        messageLineEdit->setGeometry(QRect(180, 290, 521, 61));
        clientListWidget = new QListWidget(MeetingRoomDialog);
        clientListWidget->setObjectName(QString::fromUtf8("clientListWidget"));
        clientListWidget->setGeometry(QRect(320, 30, 411, 231));
        sendButton = new QPushButton(MeetingRoomDialog);
        sendButton->setObjectName(QString::fromUtf8("sendButton"));
        sendButton->setGeometry(QRect(110, 240, 113, 30));
        broadcastButton = new QPushButton(MeetingRoomDialog);
        broadcastButton->setObjectName(QString::fromUtf8("broadcastButton"));
        broadcastButton->setGeometry(QRect(370, 820, 113, 30));
        kickButton = new QPushButton(MeetingRoomDialog);
        kickButton->setObjectName(QString::fromUtf8("kickButton"));
        kickButton->setGeometry(QRect(770, 40, 113, 30));

        retranslateUi(MeetingRoomDialog);

        QMetaObject::connectSlotsByName(MeetingRoomDialog);
    } // setupUi

    void retranslateUi(QDialog *MeetingRoomDialog)
    {
        MeetingRoomDialog->setWindowTitle(QCoreApplication::translate("MeetingRoomDialog", "Dialog", nullptr));
        statusLabel->setText(QCoreApplication::translate("MeetingRoomDialog", "statusLabel", nullptr));
        clientCountLabel->setText(QCoreApplication::translate("MeetingRoomDialog", "clientCountLabel", nullptr));
        startMeetingButton->setText(QCoreApplication::translate("MeetingRoomDialog", "startMeetingButton", nullptr));
        closeMeetingButton->setText(QCoreApplication::translate("MeetingRoomDialog", "closeMeetingButton", nullptr));
        sendButton->setText(QCoreApplication::translate("MeetingRoomDialog", "\345\217\221\351\200\201", nullptr));
        broadcastButton->setText(QCoreApplication::translate("MeetingRoomDialog", "\345\271\277\346\222\255", nullptr));
        kickButton->setText(QCoreApplication::translate("MeetingRoomDialog", "\350\270\242\345\207\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MeetingRoomDialog: public Ui_MeetingRoomDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEETINGROOMDIALOG_H
