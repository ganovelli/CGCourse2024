TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
INCLUDEPATH +=../../3dparty
LIBS += -lglfw -lGLEW
SOURCES += \
    ../../src/code_05_my_first_car/main_car.cpp

