QT += core gui widgets sql testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= console
CONFIG += windows

TARGET = The_Lab
TEMPLATE = app

# Disable deprecated APIs before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    logicandsettings.cpp \
    login.cpp \
    mainwindow.cpp \
    setupUI.cpp \
    theme.cpp \
    tictactoe.cpp

HEADERS += \
    mainwindow.h \
    tictactoe.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resource.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Windows specific settings to ensure GUI application
win32 {
    CONFIG += windows
    CONFIG -= console
}

# Test Configuration for separate build
CONFIG(build_tests) {
    TARGET = test_runner
    SOURCES -= tictactoe.cpp
    SOURCES += test_tictactoe.cpp
}
