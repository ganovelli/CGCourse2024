TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
INCLUDEPATH +=../../3dparty
LIBS += -lglfw -lGLEW
SOURCES += ../../src/code_04_transformations_pipeline/main_transf_pipeline.cpp
