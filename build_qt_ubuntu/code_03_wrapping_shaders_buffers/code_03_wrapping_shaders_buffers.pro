TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lglfw -lGLEW
SOURCES += \
    ../../src/code_03_wrapping_shaders_buffers/main_indexed.cpp
