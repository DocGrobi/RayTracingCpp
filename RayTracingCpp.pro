#-------------------------------------------------
#
# Project created by QtCreator 2017-03-28T11:27:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport multimedia

TARGET   = RayTracingCpp
TEMPLATE = app


SOURCES  += main.cpp\
            objreader.cpp \
            objwriter.cpp \
            raytracing.cpp \
            fonction.cpp \
            physic.cpp \
            rir.cpp \
            qcustomplot.cpp \
            plotwindow.cpp \
            octree.cpp \
            audio.cpp \
    mainwindow.cpp \
    fftext.cpp \
    fftlib.cpp \
    matlib.cpp \
    XFir.cpp \
    XHRir.cpp \
    XUtils.cpp \
    XSpatFir.cpp


HEADERS  += mainwindow.h \
            objreader.h \
            objwriter.h \
            raytracing.h \
            fonction.h \
            physic.h \
            rir.h \
            qcustomplot.h \
            plotwindow.h \
            octree.h \
            audio.h \
    fftext.h \
    fftlib.h \
    matlib.h \
    XFir.hpp \
    XHRir.hpp \
    XUtils.hpp \
    XSpatFir.hpp \



FORMS    += mainwindow.ui \
            plotwindow.ui



# remove possible other optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

# add the desired -O3 if not present
QMAKE_CXXFLAGS_RELEASE *= -O3

