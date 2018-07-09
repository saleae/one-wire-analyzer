cmake_minimum_required (VERSION 3.11)
project(one_wire_analyzer)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/OneWireAnalyzer.cpp
src/OneWireAnalyzer.h
src/OneWireAnalyzerResults.cpp
src/OneWireAnalyzerResults.h
src/OneWireAnalyzerSettings.cpp
src/OneWireAnalyzerSettings.h
src/OneWireSimulationDataGenerator.cpp
src/OneWireSimulationDataGenerator.h
)

add_library(one_wire_analyzer SHARED ${SOURCES})
target_link_libraries(one_wire_analyzer PRIVATE Saleae::AnalyzerSDK)

if(MSVC)
    install(TARGETS one_wire_analyzer RUNTIME DESTINATION "Analyzers")
else()
    install(TARGETS one_wire_analyzer LIBRARY DESTINATION "Analyzers")
endif()

set_post_build_destination(one_wire_analyzer)
