TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lglfw -lGLEW
SOURCES += \
        ../../src/code_02_my_first_triangle/main_indexed.cpp
