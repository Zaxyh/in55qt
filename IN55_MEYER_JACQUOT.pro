QT       += core gui widgets

TARGET = IN55_MEYER_JACQUOT
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwidget.cpp \
    src/geometryengine.cpp \
    src/camera.cpp \
    src/md5parser.cpp \
    src/md5mesh.cpp \
    src/md5anim.cpp \
    src/md5skeleton.cpp \
    src/lightindicator.cpp

HEADERS += \
    src/mainwidget.h \
    src/geometryengine.h \
    src/camera.h \
    src/md5parser.h \
    src/md5mesh.h \
    src/md5anim.h \
    src/md5skeleton.h \
    src/lightindicator.h

RESOURCES += \
    ressources/shaders/shaders.qrc \
    ressources/images/images.qrc
