/********************************************************************************
** Form generated from reading UI file 'ticketsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TICKETSDIALOG_H
#define UI_TICKETSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TicketsDialog
{
public:
    QTableView *tableViewTickets;
    QLabel *label;
    QPushButton *pushButton;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *refreshButton;
    QPushButton *changeButton;
    QPushButton *deleteButton;

    void setupUi(QDialog *TicketsDialog)
    {
        if (TicketsDialog->objectName().isEmpty())
            TicketsDialog->setObjectName(QString::fromUtf8("TicketsDialog"));
        TicketsDialog->resize(802, 608);
        tableViewTickets = new QTableView(TicketsDialog);
        tableViewTickets->setObjectName(QString::fromUtf8("tableViewTickets"));
        tableViewTickets->setGeometry(QRect(30, 60, 741, 451));
        label = new QLabel(TicketsDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 10, 80, 21));
        pushButton = new QPushButton(TicketsDialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(690, 0, 113, 30));
        horizontalLayoutWidget = new QWidget(TicketsDialog);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(140, 520, 491, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        refreshButton = new QPushButton(horizontalLayoutWidget);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));

        horizontalLayout->addWidget(refreshButton);

        changeButton = new QPushButton(horizontalLayoutWidget);
        changeButton->setObjectName(QString::fromUtf8("changeButton"));

        horizontalLayout->addWidget(changeButton);

        deleteButton = new QPushButton(horizontalLayoutWidget);
        deleteButton->setObjectName(QString::fromUtf8("deleteButton"));

        horizontalLayout->addWidget(deleteButton);


        retranslateUi(TicketsDialog);

        QMetaObject::connectSlotsByName(TicketsDialog);
    } // setupUi

    void retranslateUi(QDialog *TicketsDialog)
    {
        TicketsDialog->setWindowTitle(QCoreApplication::translate("TicketsDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("TicketsDialog", "\345\267\245\345\215\225\345\210\227\350\241\250", nullptr));
        pushButton->setText(QCoreApplication::translate("TicketsDialog", "\345\205\263\351\227\255", nullptr));
        refreshButton->setText(QCoreApplication::translate("TicketsDialog", "\345\210\267\346\226\260", nullptr));
        changeButton->setText(QCoreApplication::translate("TicketsDialog", "\344\277\256\346\224\271", nullptr));
        deleteButton->setText(QCoreApplication::translate("TicketsDialog", "\346\270\205\347\251\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TicketsDialog: public Ui_TicketsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TICKETSDIALOG_H
