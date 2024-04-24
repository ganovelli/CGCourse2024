TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
LIBS +=  -L /usr/local/lib -lglfw3 -lGLEW -lpthread -ldl
LIBS += -lglfw -lGLEW
SOURCES += \
        ../../src/code_08_raster/main_raster.cpp
