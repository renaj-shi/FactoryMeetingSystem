QT       += core gui network widgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QT += multimedia multimediawidgets

DEFINES += QT_MULTIMEDIA_LIB

SOURCES += \
    fakevideothread.cpp \
    meetingdialog.cpp \
    workorderdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    registerdialog.cpp \
    maininterfacedialog.cpp \
    deviceinfo.cpp \
    devicemonitorpanel.cpp \
    accountinfodialog.cpp \
    chatdialog.cpp \
    audioprocessor.cpp

HEADERS += \
    fakevideothread.h \
    meetingdialog.h \
    workorderdialog.h \
    mainwindow.h \
    registerdialog.h \
    maininterfacedialog.h \
    deviceinfo.h \
    devicemonitorpanel.h \
    accountinfodialog.h \
    chatdialog.h \
    audioprocessor.h

FORMS += \
    mainwindow.ui \
    meetingdialog.ui \
    registerdialog.ui \
    maininterfacedialog.ui \
    devicemonitorpanel.ui \
    workorderdialog.ui \
    accountinfodialog.ui \
    chatdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Resources
RESOURCES += resources.qrc
