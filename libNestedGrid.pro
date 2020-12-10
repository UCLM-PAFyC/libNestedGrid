#-------------------------------------------------
#
# Project created by QtCreator 2016-07-10T11:09:01
#
#-------------------------------------------------
debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}

CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
    }
CONFIG(release, debug|release) {
        CONFIG -= debug release
        CONFIG += release
}

QT += widgets

QT       += xml
QT       += sql
QT       += gui

TARGET = libNestedGrid
TEMPLATE = lib

DEFINES += LIBNESTEDGRID_LIBRARY

SOURCES += \
    NestedGridTools.cpp \
    DefinitionAnalysisDialog.cpp \
    QuadkeysDialog.cpp \
    NestedGridToolsImpl.cpp \
    NestedGridProject.cpp \
    CreateLODShapefileDialog.cpp

HEADERS +=\
        libnestedgrid_global.h \
    NestedGridTools.h \
    DefinitionAnalysisDialog.h \
    QuadkeysDialog.h \
    NestedGrid_definitions.h \
    NestedGridToolsImpl.h \
    NestedGridProject.h \
    CreateLODShapefileDialog.h

DESTDIR_RELEASE= ./../../../build/release
DESTDIR_DEBUG= ./../../../build/debug
#OSGEO4W_PATH="C:\Program Files\QGIS 2.18"
OSGEO4W_PATH="C:\Program Files\QGIS 3.4"

debug{
    DESTDIR = $$DESTDIR_DEBUG
    LIBS += -L$$DESTDIR_DEBUG
}else{
    DESTDIR = $$DESTDIR_RELEASE
    LIBS += -L$$DESTDIR_RELEASE
}


#INCLUDEPATH += ../libPW
INCLUDEPATH += ../libProcessTools
INCLUDEPATH += ../libCRS
INCLUDEPATH += ../libIGDAL
INCLUDEPATH += ../libRemoteSensing

INCLUDEPATH += . $$OSGEO4W_PATH/include
LIBS += -L$$OSGEO4W_PATH/bin
#LIBS += $$OSGEO4W_PATH/lib/gsl.lib
LIBS += $$OSGEO4W_PATH/lib/proj_i.lib
LIBS += $$OSGEO4W_PATH/lib/gdal_i.lib
LIBS += $$OSGEO4W_PATH/lib/geos_c.lib

#LIBS += -lpw
LIBS += -llibProcessTools
LIBS += -llibCRS
LIBS += -llibIGDAL
LIBS += -llibRemoteSensing

FORMS += \
    DefinitionAnalysisDialog.ui \
    QuadkeysDialog.ui \
    createLODShapefileDialog.ui

#debug{
#    win32-*{

#        !contains(QMAKE_TARGET.arch, x86_64) {
#            OSGEO4W_PATH = E:/OSGeo4W_x32
#            DESTDIR = build/debug
#        } else {
#            OSGEO4W_PATH = E:/OSGEO4W_64
#            LIBS += -L$$PWD/../../libs/libPW/build64/debug #\
#            LIBS += -L$$PWD/../../libs/libCRS/build64/debug #\
#            LIBS += -L$$PWD/../../libs/libIGDAL/build64/debug #\
#            LIBS += -L$$PWD/../../libs/libRemoteSensing/build64/debug #\
#            DESTDIR = build64/debug
#        }
#    }

#}else{
#    win32-*{
#        !contains(QMAKE_TARGET.arch, x86_64) {
#            OSGEO4W_PATH = E:/OSGeo4W_x32
#            DESTDIR = build/release
#        } else {
#            OSGEO4W_PATH = E:/OSGEO4W_64
#            DESTDIR = $$DESTDIR_RELEASE
##            DESTDIR = build64/release
#        }
#    }
#}

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
