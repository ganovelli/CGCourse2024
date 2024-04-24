TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += GLM_ENABLE_EXPERIMENTAL
LIBS +=  -L /usr/local/lib -lglfw3 -lGLEW -lpthread -ldl
INCLUDEPATH += ../../3dparty ../../3dparty/imgui ../../3dparty/stb
SOURCES += \
        ../../3dparty/imgui/backends/imgui_impl_glfw.cpp \
        ../../3dparty/imgui/backends/imgui_impl_opengl3.cpp \
        ../../3dparty/imgui/imgui.cpp \
        ../../3dparty/imgui/imgui_draw.cpp \
        ../../3dparty/imgui/imgui_tables.cpp \
        ../../3dparty/imgui/imgui_widgets.cpp \
        ../../src/code_16_gpu_volume_rendering/main.cpp

DISTFILES += \
    ../../src/code_16_gpu_volume_rendering/shaders/basic.vert
