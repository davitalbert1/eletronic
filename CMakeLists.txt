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
