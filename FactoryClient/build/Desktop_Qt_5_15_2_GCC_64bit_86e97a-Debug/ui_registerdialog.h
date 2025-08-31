/********************************************************************************
** Form generated from reading UI file 'registerdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERDIALOG_H
#define UI_REGISTERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RegisterDialog
{
public:
    QWidget *centralwidget;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *togglePasswordButton;
    QPushButton *toggleConfirmPasswordButton;
    QPushButton *registerConfirmButton;
    QPushButton *backToLoginButton;

    void setupUi(QDialog *RegisterDialog)
    {
        if (RegisterDialog->objectName().isEmpty())
            RegisterDialog->setObjectName(QString::fromUtf8("RegisterDialog"));
        RegisterDialog->resize(600, 400);
        centralwidget = new QWidget(RegisterDialog);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        usernameEdit = new QLineEdit(centralwidget);
        usernameEdit->setObjectName(QString::fromUtf8("usernameEdit"));
        usernameEdit->setGeometry(QRect(150, 80, 300, 40));
        passwordEdit = new QLineEdit(centralwidget);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setGeometry(QRect(150, 150, 250, 40));
        passwordEdit->setEchoMode(QLineEdit::Password);
        confirmPasswordEdit = new QLineEdit(centralwidget);
        confirmPasswordEdit->setObjectName(QString::fromUtf8("confirmPasswordEdit"));
        confirmPasswordEdit->setGeometry(QRect(150, 220, 250, 40));
        confirmPasswordEdit->setEchoMode(QLineEdit::Password);
        togglePasswordButton = new QPushButton(centralwidget);
        togglePasswordButton->setObjectName(QString::fromUtf8("togglePasswordButton"));
        togglePasswordButton->setGeometry(QRect(410, 150, 40, 40));
        toggleConfirmPasswordButton = new QPushButton(centralwidget);
        toggleConfirmPasswordButton->setObjectName(QString::fromUtf8("toggleConfirmPasswordButton"));
        toggleConfirmPasswordButton->setGeometry(QRect(410, 220, 40, 40));
        registerConfirmButton = new QPushButton(centralwidget);
        registerConfirmButton->setObjectName(QString::fromUtf8("registerConfirmButton"));
        registerConfirmButton->setGeometry(QRect(150, 300, 120, 40));
        backToLoginButton = new QPushButton(centralwidget);
        backToLoginButton->setObjectName(QString::fromUtf8("backToLoginButton"));
        backToLoginButton->setGeometry(QRect(330, 300, 120, 40));

        retranslateUi(RegisterDialog);

        QMetaObject::connectSlotsByName(RegisterDialog);
    } // setupUi

    void retranslateUi(QDialog *RegisterDialog)
    {
        RegisterDialog->setWindowTitle(QCoreApplication::translate("RegisterDialog", "\347\224\250\346\210\267\346\263\250\345\206\214", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("RegisterDialog", "\347\224\250\346\210\267\345\220\215", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("RegisterDialog", "\345\257\206\347\240\201", nullptr));
        confirmPasswordEdit->setPlaceholderText(QCoreApplication::translate("RegisterDialog", "\347\241\256\350\256\244\345\257\206\347\240\201", nullptr));
        togglePasswordButton->setText(QCoreApplication::translate("RegisterDialog", "\360\237\221\201\357\270\217", nullptr));
        toggleConfirmPasswordButton->setText(QCoreApplication::translate("RegisterDialog", "\360\237\221\201\357\270\217", nullptr));
        registerConfirmButton->setText(QCoreApplication::translate("RegisterDialog", "\347\241\256\350\256\244\346\263\250\345\206\214", nullptr));
        backToLoginButton->setText(QCoreApplication::translate("RegisterDialog", "\350\277\224\345\233\236\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RegisterDialog: public Ui_RegisterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERDIALOG_H
