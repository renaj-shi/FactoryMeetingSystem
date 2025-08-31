/********************************************************************************
** Form generated from reading UI file 'devicedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICEDIALOG_H
#define UI_DEVICEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DeviceDialogBase
{
public:
    QPushButton *close;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *tempLabel;
    QLabel *vibrationLabel;
    QLabel *pressureLabel;
    QLabel *voltageLabel;
    QLabel *currentLabel;
    QLabel *speedLabel;
    QLabel *updateTimeLabel;
    QLabel *statusLabel;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *tempLabel_2;
    QLabel *vibrationLabel_2;
    QLabel *pressureLabel_2;
    QLabel *voltageLabel_2;
    QLabel *currentLabel_2;
    QLabel *speedLabel_2;
    QLabel *updateTimeLabel_2;
    QLabel *statusLabel_2;
    QTableView *tableHistoryView;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_3;
    QComboBox *comboBox;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QWidget *verticalLayoutWidget_4;
    QVBoxLayout *verticalLayout_4;
    QPushButton *cautionButton;
    QPushButton *deleteHistoryButton;

    void setupUi(QDialog *DeviceDialogBase)
    {
        if (DeviceDialogBase->objectName().isEmpty())
            DeviceDialogBase->setObjectName(QString::fromUtf8("DeviceDialogBase"));
        DeviceDialogBase->resize(826, 654);
        close = new QPushButton(DeviceDialogBase);
        close->setObjectName(QString::fromUtf8("close"));
        close->setGeometry(QRect(710, 0, 113, 30));
        verticalLayoutWidget = new QWidget(DeviceDialogBase);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(90, 0, 160, 271));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tempLabel = new QLabel(verticalLayoutWidget);
        tempLabel->setObjectName(QString::fromUtf8("tempLabel"));

        verticalLayout->addWidget(tempLabel);

        vibrationLabel = new QLabel(verticalLayoutWidget);
        vibrationLabel->setObjectName(QString::fromUtf8("vibrationLabel"));

        verticalLayout->addWidget(vibrationLabel);

        pressureLabel = new QLabel(verticalLayoutWidget);
        pressureLabel->setObjectName(QString::fromUtf8("pressureLabel"));

        verticalLayout->addWidget(pressureLabel);

        voltageLabel = new QLabel(verticalLayoutWidget);
        voltageLabel->setObjectName(QString::fromUtf8("voltageLabel"));

        verticalLayout->addWidget(voltageLabel);

        currentLabel = new QLabel(verticalLayoutWidget);
        currentLabel->setObjectName(QString::fromUtf8("currentLabel"));

        verticalLayout->addWidget(currentLabel);

        speedLabel = new QLabel(verticalLayoutWidget);
        speedLabel->setObjectName(QString::fromUtf8("speedLabel"));

        verticalLayout->addWidget(speedLabel);

        updateTimeLabel = new QLabel(verticalLayoutWidget);
        updateTimeLabel->setObjectName(QString::fromUtf8("updateTimeLabel"));

        verticalLayout->addWidget(updateTimeLabel);

        statusLabel = new QLabel(verticalLayoutWidget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));

        verticalLayout->addWidget(statusLabel);

        verticalLayoutWidget_2 = new QWidget(DeviceDialogBase);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(0, 0, 81, 267));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        tempLabel_2 = new QLabel(verticalLayoutWidget_2);
        tempLabel_2->setObjectName(QString::fromUtf8("tempLabel_2"));

        verticalLayout_2->addWidget(tempLabel_2);

        vibrationLabel_2 = new QLabel(verticalLayoutWidget_2);
        vibrationLabel_2->setObjectName(QString::fromUtf8("vibrationLabel_2"));

        verticalLayout_2->addWidget(vibrationLabel_2);

        pressureLabel_2 = new QLabel(verticalLayoutWidget_2);
        pressureLabel_2->setObjectName(QString::fromUtf8("pressureLabel_2"));

        verticalLayout_2->addWidget(pressureLabel_2);

        voltageLabel_2 = new QLabel(verticalLayoutWidget_2);
        voltageLabel_2->setObjectName(QString::fromUtf8("voltageLabel_2"));

        verticalLayout_2->addWidget(voltageLabel_2);

        currentLabel_2 = new QLabel(verticalLayoutWidget_2);
        currentLabel_2->setObjectName(QString::fromUtf8("currentLabel_2"));

        verticalLayout_2->addWidget(currentLabel_2);

        speedLabel_2 = new QLabel(verticalLayoutWidget_2);
        speedLabel_2->setObjectName(QString::fromUtf8("speedLabel_2"));

        verticalLayout_2->addWidget(speedLabel_2);

        updateTimeLabel_2 = new QLabel(verticalLayoutWidget_2);
        updateTimeLabel_2->setObjectName(QString::fromUtf8("updateTimeLabel_2"));

        verticalLayout_2->addWidget(updateTimeLabel_2);

        statusLabel_2 = new QLabel(verticalLayoutWidget_2);
        statusLabel_2->setObjectName(QString::fromUtf8("statusLabel_2"));

        verticalLayout_2->addWidget(statusLabel_2);

        tableHistoryView = new QTableView(DeviceDialogBase);
        tableHistoryView->setObjectName(QString::fromUtf8("tableHistoryView"));
        tableHistoryView->setGeometry(QRect(20, 280, 791, 341));
        verticalLayoutWidget_3 = new QWidget(DeviceDialogBase);
        verticalLayoutWidget_3->setObjectName(QString::fromUtf8("verticalLayoutWidget_3"));
        verticalLayoutWidget_3->setGeometry(QRect(340, 90, 151, 117));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        comboBox = new QComboBox(verticalLayoutWidget_3);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        verticalLayout_3->addWidget(comboBox);

        pushButton = new QPushButton(verticalLayoutWidget_3);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout_3->addWidget(pushButton);

        pushButton_2 = new QPushButton(verticalLayoutWidget_3);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        verticalLayout_3->addWidget(pushButton_2);

        verticalLayoutWidget_4 = new QWidget(DeviceDialogBase);
        verticalLayoutWidget_4->setObjectName(QString::fromUtf8("verticalLayoutWidget_4"));
        verticalLayoutWidget_4->setGeometry(QRect(570, 110, 160, 81));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_4);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        cautionButton = new QPushButton(verticalLayoutWidget_4);
        cautionButton->setObjectName(QString::fromUtf8("cautionButton"));

        verticalLayout_4->addWidget(cautionButton);

        deleteHistoryButton = new QPushButton(verticalLayoutWidget_4);
        deleteHistoryButton->setObjectName(QString::fromUtf8("deleteHistoryButton"));

        verticalLayout_4->addWidget(deleteHistoryButton);


        retranslateUi(DeviceDialogBase);

        QMetaObject::connectSlotsByName(DeviceDialogBase);
    } // setupUi

    void retranslateUi(QDialog *DeviceDialogBase)
    {
        DeviceDialogBase->setWindowTitle(QCoreApplication::translate("DeviceDialogBase", "Dialog", nullptr));
        close->setText(QCoreApplication::translate("DeviceDialogBase", "\345\205\263\351\227\255", nullptr));
        tempLabel->setText(QCoreApplication::translate("DeviceDialogBase", "tempLabel", nullptr));
        vibrationLabel->setText(QCoreApplication::translate("DeviceDialogBase", "vibrationLabel", nullptr));
        pressureLabel->setText(QCoreApplication::translate("DeviceDialogBase", "pressureLabel", nullptr));
        voltageLabel->setText(QCoreApplication::translate("DeviceDialogBase", "voltageLabel", nullptr));
        currentLabel->setText(QCoreApplication::translate("DeviceDialogBase", "currentLabel", nullptr));
        speedLabel->setText(QCoreApplication::translate("DeviceDialogBase", "speedLabel", nullptr));
        updateTimeLabel->setText(QCoreApplication::translate("DeviceDialogBase", "updateTimeLabel", nullptr));
        statusLabel->setText(QCoreApplication::translate("DeviceDialogBase", "statusLabel", nullptr));
        tempLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\346\270\251\345\272\246", nullptr));
        vibrationLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\346\214\257\345\212\250", nullptr));
        pressureLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\345\216\213\345\212\233", nullptr));
        voltageLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\347\224\265\345\216\213", nullptr));
        currentLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\347\224\265\346\265\201", nullptr));
        speedLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\351\200\237\345\272\246", nullptr));
        updateTimeLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\346\233\264\346\226\260\346\227\266\351\227\264", nullptr));
        statusLabel_2->setText(QCoreApplication::translate("DeviceDialogBase", "\347\212\266\346\200\201", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("DeviceDialogBase", "\346\270\251\345\272\246", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("DeviceDialogBase", "\345\216\213\345\212\233", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("DeviceDialogBase", "\346\214\257\345\212\250", nullptr));
        comboBox->setItemText(3, QCoreApplication::translate("DeviceDialogBase", "\347\224\265\346\265\201", nullptr));
        comboBox->setItemText(4, QCoreApplication::translate("DeviceDialogBase", "\347\224\265\345\216\213", nullptr));
        comboBox->setItemText(5, QCoreApplication::translate("DeviceDialogBase", "\350\275\254\351\200\237", nullptr));

        pushButton->setText(QCoreApplication::translate("DeviceDialogBase", "\346\237\245\347\234\213\345\217\230\345\214\226\350\266\213\345\212\277", nullptr));
        pushButton_2->setText(QCoreApplication::translate("DeviceDialogBase", "\345\210\267\346\226\260\345\216\206\345\217\262\346\225\260\346\215\256", nullptr));
        cautionButton->setText(QCoreApplication::translate("DeviceDialogBase", "\357\274\201\346\225\205\351\232\234\351\242\204\350\255\246\357\274\201", nullptr));
        deleteHistoryButton->setText(QCoreApplication::translate("DeviceDialogBase", "\345\210\240\351\231\244\345\216\206\345\217\262\346\225\260\346\215\256", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceDialogBase: public Ui_DeviceDialogBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICEDIALOG_H
