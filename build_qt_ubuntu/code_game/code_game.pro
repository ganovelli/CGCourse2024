TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lglfw -lGLEW
SOURCES += \
        ../../src/code_01_setup_glfw/main_01.cpp
