cmake_minimum_required(VERSION 3.20)

project(NPPAIPlugin-Ollama)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    AIProviders/*.cpp
    Http/*.cpp
    Utils/*.cpp
    DockingFeature/*.cpp
)

add_library(NPPAIPlugin-Ollama SHARED ${SOURCES})

target_include_directories(
    NPPAIPlugin-Ollama
    PRIVATE
    .
    src
    AIProviders
    Http
    Utils
    DockingFeature
)

target_link_libraries(
    NPPAIPlugin-Ollama
    winhttp
    shlwapi
)
