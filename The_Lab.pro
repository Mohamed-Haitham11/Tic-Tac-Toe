QT       += core gui
QT += core gui widgets sql testlib
QT += sql
QT += core gui widgets sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
RESOURCES = resource.qrc \
    resource.qrc
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ===============================================
# Test Configuration for a Separate Build
# ===============================================
CONFIG(build_tests) {
    TARGET = test_runner
   SOURCES -= tictactoe.cpp
    #SOURCES -= main.cpp
    SOURCES += test_tictactoe.cpp
}
