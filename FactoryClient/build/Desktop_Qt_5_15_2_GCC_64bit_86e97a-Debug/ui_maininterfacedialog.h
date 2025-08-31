/********************************************************************************
** Form generated from reading UI file 'maininterfacedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAININTERFACEDIALOG_H
#define UI_MAININTERFACEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainInterfaceDialog
{
public:
    QTabBar *tabBar;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout;
    QLabel *emptyLabel;
    QPushButton *loginButton;
    QPushButton *workOrderButton;
    QPushButton *newWorkOrderButton;
    QPushButton *workOrderListButton;
    QPushButton *communicationButton;
    QPushButton *textMessageButton;
    QPushButton *deviceMonitorButton;
    QPushButton *realTimeDataButton;
    QPushButton *deviceRecordingButton;
    QPushButton *knowledgeBaseButton;
    QPushButton *accountButton;
    QPushButton *logoutButton;

    void setupUi(QDialog *MainInterfaceDialog)
    {
        if (MainInterfaceDialog->objectName().isEmpty())
            MainInterfaceDialog->setObjectName(QString::fromUtf8("MainInterfaceDialog"));
        MainInterfaceDialog->resize(1200, 800);
        tabBar = new QTabBar(MainInterfaceDialog);
        tabBar->setObjectName(QString::fromUtf8("tabBar"));
        stackedWidget = new QStackedWidget(MainInterfaceDialog);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        verticalLayout = new QVBoxLayout(page);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        emptyLabel = new QLabel(page);
        emptyLabel->setObjectName(QString::fromUtf8("emptyLabel"));
        emptyLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(emptyLabel);

        stackedWidget->addWidget(page);
        loginButton = new QPushButton(MainInterfaceDialog);
        loginButton->setObjectName(QString::fromUtf8("loginButton"));
        workOrderButton = new QPushButton(MainInterfaceDialog);
        workOrderButton->setObjectName(QString::fromUtf8("workOrderButton"));
        newWorkOrderButton = new QPushButton(MainInterfaceDialog);
        newWorkOrderButton->setObjectName(QString::fromUtf8("newWorkOrderButton"));
        workOrderListButton = new QPushButton(MainInterfaceDialog);
        workOrderListButton->setObjectName(QString::fromUtf8("workOrderListButton"));
        communicationButton = new QPushButton(MainInterfaceDialog);
        communicationButton->setObjectName(QString::fromUtf8("communicationButton"));
        textMessageButton = new QPushButton(MainInterfaceDialog);
        textMessageButton->setObjectName(QString::fromUtf8("textMessageButton"));
        deviceMonitorButton = new QPushButton(MainInterfaceDialog);
        deviceMonitorButton->setObjectName(QString::fromUtf8("deviceMonitorButton"));
        realTimeDataButton = new QPushButton(MainInterfaceDialog);
        realTimeDataButton->setObjectName(QString::fromUtf8("realTimeDataButton"));
        deviceRecordingButton = new QPushButton(MainInterfaceDialog);
        deviceRecordingButton->setObjectName(QString::fromUtf8("deviceRecordingButton"));
        knowledgeBaseButton = new QPushButton(MainInterfaceDialog);
        knowledgeBaseButton->setObjectName(QString::fromUtf8("knowledgeBaseButton"));
        accountButton = new QPushButton(MainInterfaceDialog);
        accountButton->setObjectName(QString::fromUtf8("accountButton"));
        logoutButton = new QPushButton(MainInterfaceDialog);
        logoutButton->setObjectName(QString::fromUtf8("logoutButton"));

        retranslateUi(MainInterfaceDialog);

        QMetaObject::connectSlotsByName(MainInterfaceDialog);
    } // setupUi

    void retranslateUi(QDialog *MainInterfaceDialog)
    {
        MainInterfaceDialog->setWindowTitle(QCoreApplication::translate("MainInterfaceDialog", "\345\267\245\345\216\202\345\256\242\346\210\267\347\253\257 - \344\270\273\347\225\214\351\235\242", nullptr));
        emptyLabel->setText(QCoreApplication::translate("MainInterfaceDialog", "\346\227\240\345\267\245\344\275\234\344\273\273\345\212\241\346\255\243\345\234\250\350\277\233\350\241\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainInterfaceDialog: public Ui_MainInterfaceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAININTERFACEDIALOG_H
