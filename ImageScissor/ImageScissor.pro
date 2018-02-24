#-------------------------------------------------
#
# Project created by QtCreator 2018-02-14T10:47:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageScissor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += /usr/local/Cellar/opencv/3.4.0_1/include

SOURCES += \
        main.cpp \
        imagescissor.cpp \
    imagelabel.cpp \
    imagenode.cpp

HEADERS += \
        imagescissor.h \
    imagelabel.h \
    imagenode.h

FORMS += \
        imagescissor.ui

LIBS += -L/usr/local/Cellar/opencv/3.4.0_1/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui
