TEMPLATE = app
TARGET = ExpertClientChat

QT += core gui widgets network charts
CONFIG += c++17

SOURCES += \
    chatclient.cpp \
    knowledgeclient.cpp \
    knowledgeviewdialog.cpp \
    main.cpp \
    tcpclient.cpp \
    authmanager.cpp \
    ticketsclient.cpp \
    ticketsviewdialog.cpp \
    workordermanager.cpp \
    telemetryclient.cpp \
    logindialog.cpp \
    sessionwidget.cpp \
    mainwindow.cpp

HEADERS += \
    chatclient.h \
    knowledgeclient.h \
    knowledgeviewdialog.h \
    tcpclient.h \
    authmanager.h \
    ticketsclient.h \
    ticketsviewdialog.h \
    workordermanager.h \
    telemetryclient.h \
    logindialog.h \
    sessionwidget.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
