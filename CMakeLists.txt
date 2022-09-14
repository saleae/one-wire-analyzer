cmake_minimum_required (VERSION 3.11)
project(one_wire_analyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
