TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
LIBS +=  -L /usr/local/lib -lglfw3 -lGLEW -lpthread -ldl
SOURCES += \
        ../../src/code_07_manipulation/main_manipulation.cpp
