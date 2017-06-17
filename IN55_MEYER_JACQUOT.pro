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
    src/md5anim.cpp

HEADERS += \
    src/mainwidget.h \
    src/geometryengine.h \
    src/camera.h \
    src/md5parser.h \
    src/md5mesh.h \
    src/md5anim.h

RESOURCES += \
    ressources/shaders.qrc
