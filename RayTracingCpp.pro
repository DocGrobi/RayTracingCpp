#-------------------------------------------------
#
# Project created by QtCreator 2017-03-28T11:27:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RayTracingCpp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    objreader.cpp \
    objwriter.cpp \
    raytracing.cpp


HEADERS  += mainwindow.h \
    objreader.h \
    objwriter.h \
    raytracing.h


FORMS    += mainwindow.ui



