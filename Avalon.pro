#-------------------------------------------------
#
# Project created by QtCreator 2017-01-13T20:36:34
#
#-------------------------------------------------

QT       += core gui opengl
QT       += widgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:RC_ICONS += geass.ico

TARGET = Avalon
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Desktop
# INCLUDEPATH += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\Armadillo
# INCLUDEPATH += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\Armadillo\include\armadillo_bits
# INCLUDEPATH += C:\Qt\Qt5.6.0\Tools\QtCreator\lib
# INCLUDEPATH += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\Armadillo\include

#Dekstop
# LIBS += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\lapack_win64_MT.lib
# LIBS += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\libopenblas.a
# LIBS += C:\Qt\Qt5.6.0\Tools\QtCreator\lib\lapack_win64_MT.dll

#Laptop
#For Armadillo.
INCLUDEPATH += C:\Qtgg\Tools\QtCreator\lib\Armadillo
INCLUDEPATH += C:\Qtgg\Tools\QtCreator\lib\Armadillo\include\armadillo_bits
#INCLUDEPATH += C:\Qtgg\Tools\QtCreator\lib
INCLUDEPATH += C:\Qtgg\Tools\QtCreator\lib\Armadillo\include
#For reBlas and Lapack.
INCLUDEPATH += C:\Qtgg\Lapack
INCLUDEPATH += C:\MinGW\bin

#Libraries suck and don't work.  Eigen.
INCLUDEPATH += C:\Qtgg\Lapack\EigenTop\eigen-eigen-da9b4e14c255\Eigen



#Laptop
#LIBS += C:\Qtgg\Tools\QtCreator\lib\lapack_win64_MT.lib
LIBS += C:\Qtgg\Tools\QtCreator\lib\libopenblas.a
LIBS += C:\Qtgg\Lapack\libblas.lib
LIBS += C:\Qtgg\Lapack\liblapack.lib



SOURCES += main.cpp\
        mainwindow.cpp \
    utilities.cpp \
    elements.cpp \
    globals.cpp \
    avalonmatrices.cpp

HEADERS  += mainwindow.h \
    utilities.h \
    elements.h \
    globals.h \
    avalonmatrices.h

FORMS    += mainwindow.ui
