TEMPLATE = app
TARGET = ExpertClientChat

QT += core gui widgets network charts multimedia multimediawidgets
CONFIG += c++17

SOURCES += \
    chatclient.cpp \
    main.cpp \
    meetingdialog.cpp \
    tcpclient.cpp \
    authmanager.cpp \
    ticketsclient.cpp \
    ticketsviewdialog.cpp \
    workordermanager.cpp \
    telemetryclient.cpp \
    logindialog.cpp \
    sessionwidget.cpp \
    mainwindow.cpp \
    audioprocessor.cpp \
    videoprocessor.cpp

HEADERS += \
    chatclient.h \
    meetingdialog.h \
    tcpclient.h \
    authmanager.h \
    ticketsclient.h \
    ticketsviewdialog.h \
    workordermanager.h \
    telemetryclient.h \
    logindialog.h \
    sessionwidget.h \
    mainwindow.h \
    audioprocessor.h \
    videoprocessor.h

FORMS += \
    mainwindow.ui \
    meetingdialog.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
