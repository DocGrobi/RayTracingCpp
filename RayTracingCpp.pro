#-------------------------------------------------
#
# Project created by QtCreator 2017-03-28T11:27:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = RayTracingCpp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    objreader.cpp \
    objwriter.cpp \
    raytracing.cpp \
    fonction.cpp \
    physic.cpp \
    rir.cpp \
    qcustomplot.cpp \
    plotwindow.cpp \
    octree.cpp


HEADERS  += mainwindow.h \
    objreader.h \
    objwriter.h \
    raytracing.h \
    fonction.h \
    physic.h \
    rir.h \
    qcustomplot.h \
    plotwindow.h \
    octree.h


FORMS    += mainwindow.ui \
    plotwindow.ui



