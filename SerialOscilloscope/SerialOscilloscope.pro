#-------------------------------------------------
#
# Project created by QtCreator 2016-10-19T23:33:05
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SerialOscilloscope
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui


#LIBS += -L "E:\Qt\Qt5.5.1\5.5\mingw492_32\lib" -lqwt
#INCLUDEPATH += "E:\Qt\Qt5.5.1\5.5\mingw492_32\include\qwt"
#include (F:\ISE\Qt\qwt-6.1.3\qwt.prf)
