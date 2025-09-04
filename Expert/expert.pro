TEMPLATE = app
TARGET = ExpertClientChat

QT += core gui widgets network charts multimedia multimediawidgets
CONFIG += c++17

SOURCES += \
    chatclient.cpp \
    chatmessagewidget.cpp \
    fakevideothread.cpp \
    ffmpegrecorder.cpp \
    main.cpp \
    meetingdialog.cpp \
    registerdialog.cpp \
    tcpclient.cpp \
    authmanager.cpp \
    ticketsclient.cpp \
    ticketspagewidget.cpp \
    ticketsviewdialog.cpp \
    welcomewidget.cpp \
    workordermanager.cpp \
    telemetryclient.cpp \
    logindialog.cpp \
    sessionwidget.cpp \
    mainwindow.cpp \
    audioprocessor.cpp \
    videoprocessor.cpp

HEADERS += \
    chatclient.h \
    chatmessagewidget.h \
    fakevideothread.h \
    ffmpegrecorder.h \
    meetingdialog.h \
    registerdialog.h \
    tcpclient.h \
    authmanager.h \
    ticketsclient.h \
    ticketspagewidget.h \
    ticketsviewdialog.h \
    welcomewidget.h \
    workordermanager.h \
    telemetryclient.h \
    logindialog.h \
    sessionwidget.h \
    mainwindow.h \
    audioprocessor.h \
    videoprocessor.h

FORMS += \
    mainwindow.ui \
    meetingdialog.ui \
    sessionwidget.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
