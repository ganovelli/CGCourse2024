TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
LIBS +=  -L /usr/local/lib -lglfw3 -lGLEW -lpthread -ldl
INCLUDEPATH += ../../3dparty
DEFINES += GLM_ENABLE_EXPERIMENTAL
SOURCES += \
        ../../src/code_09_gltfloader/main_gltf_test.cpp

