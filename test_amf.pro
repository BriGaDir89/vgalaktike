#-------------------------------------------------
#
# Project created by QtCreator 2016-02-03T20:34:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_amf
TEMPLATE = app

LIBS += -lws2_32
INCLUDEPATH += C:/OpenSSL-Win32/include
DEPENDPATH += C:/OpenSSL-Win32/include
LIBS += -LC:/OpenSSL-Win32/lib/ -llibeay32
LIBS += -LC:/OpenSSL-Win32/lib/ -lssleay32

SOURCES += main.cpp\
        mainwindow.cpp \
    amf.cpp

HEADERS  += mainwindow.h \
    amf.h

FORMS    += mainwindow.ui
