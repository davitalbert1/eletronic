cmake_minimum_required(VERSION 3.15)
project(LogicGateSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuração para Windows: simulador com console acessível
# (se WIN32_EXECUTABLE estiver ativo, console de teste fica invisível no terminal)
# if(WIN32)
#     set(CMAKE_WIN32_EXECUTABLE TRUE)
# endif()

# Coletar arquivos do sistema principal e testes
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# Testes unitários para execução no executável principal e runner dedicado
set(TEST_MODULE_SOURCES
    "tests/TestLogicGates.cpp"
    "tests/TestJson.cpp"
    "tests/TestCircuitLanguage.cpp"
    "tests/TestSynthesizer.cpp"
    "tests/TestChipsAndUndo.cpp"
    "tests/TestImporter.cpp"
)

# Criar executável do sistema principal (com testes integrados via --test ou aba de testes)
add_executable(simulator ${SOURCES} ${TEST_MODULE_SOURCES})

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

# Executável independente de testes para CI/CD ou linha de comando direta
add_executable(run_tests
    tests/test_main.cpp
    ${TEST_MODULE_SOURCES}
    src/Gate.cpp
    src/GateRegistry.cpp
    src/Wire.cpp
    src/UndoRedoManager.cpp
    src/CircuitLanguageParser.cpp
    src/CircuitSynthesizer.cpp
    src/DigitalLogicSimImporter.cpp
    src/PdfExporter.cpp
    src/ImageExporter.cpp
    src/ConfirmationDialog.cpp
    src/chip/Json.cpp
    src/chip/Circuit.cpp
    src/chip/CustomChip.cpp
    src/chip/ChipDefinition.cpp
    src/chip/ChipLibrary.cpp
    src/chip/ChipFactory.cpp
)

target_include_directories(run_tests PRIVATE include)
target_link_libraries(run_tests PRIVATE SFML::Graphics SFML::Window SFML::System)
if(WIN32)
    target_link_libraries(run_tests PRIVATE opengl32 gdi32 dwmapi)
endif()
