QT       += core gui sql network charts serialport widgets multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += multimedia multimediawidgets

DEFINES += QT_MULTIMEDIA_LIB

SOURCES += \
    devicedialog.cpp \
    deviceworker.cpp \
    main.cpp \
    mainwindow.cpp \
    meetingroomdialog.cpp \
    ticketsdialog.cpp \
    videothread.cpp \
    audioprocessor.cpp

HEADERS += \
    devicedialog.h \
    deviceworker.h \
    mainwindow.h \
    meetingroomdialog.h \
    ticketsdialog.h \
    videothread.h \
    audioprocessor.h

FORMS += \
    devicedialog.ui \
    mainwindow.ui \
    meetingroomdialog.ui \
    ticketsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
