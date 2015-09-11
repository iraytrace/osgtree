#-------------------------------------------------
#
# Project created by QtCreator 2015-09-08T16:12:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = osgtree
TEMPLATE = app
LIBS += -losg -losgDB -losgUtil -losgViewer -losgGA

SOURCES += main.cpp\
        MainWindow.cpp \
    OsgItemModel.cpp \
    OsgTreeView.cpp \
    OsgTreeForm.cpp \
    OsgView.cpp \
    ViewingCore.cpp

HEADERS  += MainWindow.h \
    OsgItemModel.h \
    OsgTreeView.h \
    VariantPtr.h \
    OsgTreeForm.h \
    OsgView.h \
    ViewingCore.h

FORMS    += MainWindow.ui \
    OsgTreeForm.ui
