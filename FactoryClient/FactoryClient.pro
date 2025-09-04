QT       += core gui network widgets multimedia multimediawidgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    meetingdialog.cpp \
    workorderdialog.cpp \
    workorderlistdialog.cpp \
    workorderdetaildialog.cpp \
    main.cpp \
    mainwindow.cpp \
    registerdialog.cpp \
    maininterfacedialog.cpp \
    deviceinfo.cpp \
    devicemonitorpanel.cpp \
    accountinfodialog.cpp \
    chatdialog.cpp \
    audioprocessor.cpp \
    videoprocessor.cpp \
    devicelinechartdialog.cpp \
    screenrecorder.cpp \
    screenrecordingwidget.cpp \

HEADERS += \
    meetingdialog.h \
    workorderdialog.h \
    workorderlistdialog.h \
    workorderdetaildialog.h \
    mainwindow.h \
    registerdialog.h \
    maininterfacedialog.h \
    deviceinfo.h \
    devicemonitorpanel.h \
    accountinfodialog.h \
    chatdialog.h \
    audioprocessor.h \
    videoprocessor.h \
    devicelinechartdialog.h \
    screenrecorder.h \
    screenrecordingwidget.h \

FORMS += \
    mainwindow.ui \
    meetingdialog.ui \
    registerdialog.ui \
    maininterfacedialog.ui \
    devicemonitorpanel.ui \
    workorderdialog.ui \
    workorderlistdialog.ui \
    workorderdetaildialog.ui \
    accountinfodialog.ui \
    chatdialog.ui \
    devicelinechartdialog.ui \
    screenrecordingwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Resources
RESOURCES += resources.qrc
