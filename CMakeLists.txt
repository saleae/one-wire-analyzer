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

add_analyzer_plugin(one_wire_analyzer SOURCES ${SOURCES})
