cmake_minimum_required(VERSION 3.15)
project(LogicGateSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuração para Windows - remove janela do console
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()

# Coletar automaticamente todos os arquivos .cpp da pasta src
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# Criar executável
add_executable(simulator ${SOURCES})

# Diretórios de include
target_include_directories(simulator PRIVATE include)

# Linkar bibliotecas SFML
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)
target_link_libraries(simulator PRIVATE SFML::Graphics SFML::Window SFML::System)

# Adicionar ImGui
target_include_directories(simulator PRIVATE
    vendor/imgui
    vendor/imgui/backends
    vendor/imgui/misc/cpp
    vendor/glad/include
)

# Arquivos do ImGui
target_sources(simulator PRIVATE
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/misc/cpp/imgui_stdlib.cpp
    vendor/imgui/backends/imgui_impl_win32.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
    vendor/glad/src/glad.c
)

# Linkar bibliotecas do OpenGL e Win32
if(WIN32)
    target_link_libraries(simulator PRIVATE opengl32 gdi32 dwmapi)
endif()
