#-------------------------------------------------
Z#
# Project created by QtCreator 2013-07-12T17:15:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = app
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
#    decomposedfont.cppZ \
    composedglyph.cpp

HEADERS  += mainwindow.h \
#    view.h \
#    combination.h
#    decomposedfont.h \
    composedglyph.h

FORMS    += mainwindow.ui

macx {
#  ICON = ./transtype4.icns
#  QMAKE_INFO_PLIST = ./info_mac.plist
  FONTGATE = ../../FontGate

  DEFINES += MACOS
  DEFINES += MAC_OS

  LIBS += -framework Carbon -framework Cocoa
  LIBS += -lz
  LIBS += /usr/local/lib/libpng15.a
}

win32 {
# put here correct relative path to FG on windows

  FONTGATE = d:/qtProjects/flibs/FontGate/

  DEFINES += WIN32
  DEFINES += _WIN32

  LIBS += -lzlib
  LIBS += -lpng
}


CONFIG(debug,debug|release) {
    message( debug )
    OBJECTS_DIR = ./qt_build_debug
    DESTDIR = ./qt_debug

    LIBS += -L$$FONTGATE/fontgate/qt_debug
    LIBS += -L$$FONTGATE/dependences/libs/debug
#    LIBS += -L../decomposerLib/qt_debug
#    LIBS += -lpng
} else {
    message( release )
    OBJECTS_DIR = ./qt_build_release
    DESTDIR = ./qt_release

    LIBS += -L$$FONTGATE/fontgate/qt_release
    LIBS += -L$$FONTGATE/dependences/libs/release
#    LIBS += -L../decomposerLib/qt_release
}


LIBS += -L$$FONTGATE/dependences/libs
LIBS += -L$$FONTGATE/dependences/dependences/libs/debug

LIBS += -lfontgate

LIBS += -lhotconv \
  -ldynarr \
  -lctutil \
  -lac

LIBS += \
    #-licui18n \
    #-licuuc \
    #-licule \
    #-licudata \

#LIBS += -ldecomposerLib

INCLUDEPATH += $$FONTGATE/blend/
#INCLUDEPATH += ../decomposerLib/
INCLUDEPATH += $$FONTGATE/opentype/
INCLUDEPATH += $$FONTGATE/headers/
INCLUDEPATH += $$FONTGATE/io/
INCLUDEPATH += $$FONTGATE/xml/
INCLUDEPATH += $$FONTGATE/svg/
INCLUDEPATH += $$FONTGATE/dependences/ft/
INCLUDEPATH += $$FONTGATE/dependences/Expat/Source/lib/
INCLUDEPATH += $$FONTGATE/curve-convert/

#to_curve.h


HEADERS += $$FONTGATE/headers/errors.h \
$$FONTGATE/opentype/tebdt.h \
 $$FONTGATE/opentype/tsbix.h \
    $$FONTGATE/opentype/tsvg.h \
    $$FONTGATE/io/png_image.h \
    $$FONTGATE/dependences/Expat/Source/lib/expat.h\
    #$$FONTGATE/



SOURCES +=  $$FONTGATE/raster/raster.cpp \
    $$FONTGATE/opentype/tebdt.cpp \
    $$FONTGATE/opentype/tsvg.cpp

