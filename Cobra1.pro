#-------------------------------------------------
#
# Project created by QtCreator 2015-07-29T20:53:17
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Cobra1
TEMPLATE = app


SOURCES += main.cpp\
    atadisk.cpp \
    audioay.cpp \
    configfile.cpp \
        mainwindow.cpp \
    eden.cpp \
    audioplayer.cpp \
    audioplayerthread.cpp \
    screen.cpp \
    keyboard.cpp \
    cpumem.cpp \
    appcore.cpp \
    tapeplayer.cpp \
    tape.cpp \
    screenkeyb.cpp \
    helpsettings.cpp

HEADERS  += mainwindow.h \
    atadisk.h \
    audioay.h \
    binary.h \
    configfile.h \
    eden.h \
    audioplayer.h \
    audioplayerthread.h \
    screen.h \
    keyboard.h \
    cpumem.h \
    appcore.h \
    tapeplayer.h \
    tape.h \
    screenkeyb.h \
    helpsettings.h \
    objmem.h

FORMS    += mainwindow.ui \
    tapeplayer.ui \
    screenkeyb.ui \
    helpsettings.ui

CONFIG += c++11
