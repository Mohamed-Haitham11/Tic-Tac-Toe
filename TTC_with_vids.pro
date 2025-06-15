QT       += core gui
QT += core gui widgets
QT += sql
QT += core gui widgets sql multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    logicandsettings.cpp \
    login.cpp \
    main.cpp \
    setupUI.cpp \
    theme.cpp \
    tictactoe.cpp

HEADERS += \
    tictactoe.h

FORMS += \
    mainwindow.ui
RESOURCES = resource.qrc \
    resource.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
