TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH +=../../3dparty
DEFINES += GLM_ENABLE_EXPERIMENTAL
LIBS += -lglfw -lGLEW
SOURCES += \
        ../../src/code_06_rotation_interpolation/main_rot.cpp
