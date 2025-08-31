/********************************************************************************
** Form generated from reading UI file 'workorderdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORKORDERDIALOG_H
#define UI_WORKORDERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WorkOrderDialog
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel;
    QWidget *formWidget;
    QVBoxLayout *formLayout;
    QHBoxLayout *titleInputLayout;
    QLabel *titleInputLabel;
    QLineEdit *titleEdit;
    QHBoxLayout *priorityLayout;
    QLabel *priorityLabel;
    QComboBox *priorityComboBox;
    QHBoxLayout *descriptionLayout;
    QLabel *descriptionLabel;
    QTextEdit *descriptionTextEdit;
    QHBoxLayout *buttonLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *confirmButton;

    void setupUi(QDialog *WorkOrderDialog)
    {
        if (WorkOrderDialog->objectName().isEmpty())
            WorkOrderDialog->setObjectName(QString::fromUtf8("WorkOrderDialog"));
        WorkOrderDialog->resize(600, 500);
        centralwidget = new QWidget(WorkOrderDialog);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        titleLabel = new QLabel(centralwidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #1e293b;"));
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel);

        formWidget = new QWidget(centralwidget);
        formWidget->setObjectName(QString::fromUtf8("formWidget"));
        formWidget->setStyleSheet(QString::fromUtf8("background-color: #f8fafc; border-radius: 8px;"));
        formLayout = new QVBoxLayout(formWidget);
        formLayout->setSpacing(15);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setContentsMargins(20, 20, 20, 20);
        titleInputLayout = new QHBoxLayout();
        titleInputLayout->setObjectName(QString::fromUtf8("titleInputLayout"));
        titleInputLabel = new QLabel(formWidget);
        titleInputLabel->setObjectName(QString::fromUtf8("titleInputLabel"));
        titleInputLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; color: #475569;"));
        titleInputLabel->setMinimumSize(QSize(100, 0));

        titleInputLayout->addWidget(titleInputLabel);

        titleEdit = new QLineEdit(formWidget);
        titleEdit->setObjectName(QString::fromUtf8("titleEdit"));
        titleEdit->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 8px; border: 1px solid #cbd5e1; border-radius: 4px;"));

        titleInputLayout->addWidget(titleEdit);


        formLayout->addLayout(titleInputLayout);

        priorityLayout = new QHBoxLayout();
        priorityLayout->setObjectName(QString::fromUtf8("priorityLayout"));
        priorityLabel = new QLabel(formWidget);
        priorityLabel->setObjectName(QString::fromUtf8("priorityLabel"));
        priorityLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; color: #475569;"));
        priorityLabel->setMinimumSize(QSize(100, 0));

        priorityLayout->addWidget(priorityLabel);

        priorityComboBox = new QComboBox(formWidget);
        priorityComboBox->addItem(QString());
        priorityComboBox->addItem(QString());
        priorityComboBox->addItem(QString());
        priorityComboBox->addItem(QString());
        priorityComboBox->setObjectName(QString::fromUtf8("priorityComboBox"));
        priorityComboBox->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 8px; border: 1px solid #cbd5e1; border-radius: 4px;"));

        priorityLayout->addWidget(priorityComboBox);


        formLayout->addLayout(priorityLayout);

        descriptionLayout = new QHBoxLayout();
        descriptionLayout->setObjectName(QString::fromUtf8("descriptionLayout"));
        descriptionLabel = new QLabel(formWidget);
        descriptionLabel->setObjectName(QString::fromUtf8("descriptionLabel"));
        descriptionLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; color: #475569;"));
        descriptionLabel->setMinimumSize(QSize(100, 0));
        descriptionLabel->setAlignment(Qt::AlignTop);

        descriptionLayout->addWidget(descriptionLabel);

        descriptionTextEdit = new QTextEdit(formWidget);
        descriptionTextEdit->setObjectName(QString::fromUtf8("descriptionTextEdit"));
        descriptionTextEdit->setStyleSheet(QString::fromUtf8("font-size: 14px; padding: 8px; border: 1px solid #cbd5e1; border-radius: 4px;"));
        descriptionTextEdit->setMinimumSize(QSize(0, 120));

        descriptionLayout->addWidget(descriptionTextEdit);


        formLayout->addLayout(descriptionLayout);


        verticalLayout->addWidget(formWidget);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(centralwidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #e2e8f0; color: #1e293b; font-size: 14px; padding: 10px 24px; border-radius: 4px; border: none; }\n"
"QPushButton:hover { background-color: #cbd5e1; }\n"
"QPushButton:pressed { background-color: #94a3b8; }"));

        buttonLayout->addWidget(cancelButton);

        confirmButton = new QPushButton(centralwidget);
        confirmButton->setObjectName(QString::fromUtf8("confirmButton"));
        confirmButton->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3b82f6; color: white; font-size: 14px; padding: 10px 24px; border-radius: 4px; border: none; }\n"
"QPushButton:hover { background-color: #2563eb; }\n"
"QPushButton:pressed { background-color: #1d4ed8; }"));

        buttonLayout->addWidget(confirmButton);


        verticalLayout->addLayout(buttonLayout);


        retranslateUi(WorkOrderDialog);

        QMetaObject::connectSlotsByName(WorkOrderDialog);
    } // setupUi

    void retranslateUi(QDialog *WorkOrderDialog)
    {
        WorkOrderDialog->setWindowTitle(QCoreApplication::translate("WorkOrderDialog", "\346\226\260\345\273\272\345\267\245\345\215\225", nullptr));
        titleLabel->setText(QCoreApplication::translate("WorkOrderDialog", "\345\267\245\345\215\225\344\277\241\346\201\257", nullptr));
        titleInputLabel->setText(QCoreApplication::translate("WorkOrderDialog", "\345\267\245\345\215\225\346\240\207\351\242\230:", nullptr));
        titleEdit->setPlaceholderText(QCoreApplication::translate("WorkOrderDialog", "\350\257\267\350\276\223\345\205\245\345\267\245\345\215\225\346\240\207\351\242\230", nullptr));
        priorityLabel->setText(QCoreApplication::translate("WorkOrderDialog", "\347\264\247\346\200\245\347\250\213\345\272\246:", nullptr));
        priorityComboBox->setItemText(0, QCoreApplication::translate("WorkOrderDialog", "\344\275\216", nullptr));
        priorityComboBox->setItemText(1, QCoreApplication::translate("WorkOrderDialog", "\344\270\255", nullptr));
        priorityComboBox->setItemText(2, QCoreApplication::translate("WorkOrderDialog", "\351\253\230", nullptr));
        priorityComboBox->setItemText(3, QCoreApplication::translate("WorkOrderDialog", "\347\264\247\346\200\245", nullptr));

        descriptionLabel->setText(QCoreApplication::translate("WorkOrderDialog", "\345\267\245\345\215\225\346\217\217\350\277\260:", nullptr));
        descriptionTextEdit->setPlaceholderText(QCoreApplication::translate("WorkOrderDialog", "\350\257\267\350\257\246\347\273\206\346\217\217\350\277\260\345\267\245\345\215\225\345\206\205\345\256\271...", nullptr));
        cancelButton->setText(QCoreApplication::translate("WorkOrderDialog", "\345\217\226\346\266\210", nullptr));
        confirmButton->setText(QCoreApplication::translate("WorkOrderDialog", "\347\241\256\350\256\244\345\210\233\345\273\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WorkOrderDialog: public Ui_WorkOrderDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORKORDERDIALOG_H
