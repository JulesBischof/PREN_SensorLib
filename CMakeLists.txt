# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(SensorLib C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(SensorLib main.cpp )

pico_set_program_name(SensorLib "SensorLib")
pico_set_program_version(SensorLib "0.1")

# Generate PIO header
pico_generate_pio_header(SensorLib ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(SensorLib 1)
pico_enable_stdio_usb(SensorLib 1)

# Add the standard library to the build
target_link_libraries(SensorLib
        pico_stdlib)

# Include directories for header files
target_include_directories(SensorLib PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# add subdirectories
add_subdirectory(PhotoSensor)
add_subdirectory(MPU6050)
add_subdirectory(TFLuna)
add_subdirectory(TMC5240)

# Add any user requested libraries
target_link_libraries(SensorLib 
        hardware_spi
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_clocks
        
        PhotoSensorArray
        MPU6050
        TFLuna
        TMC5240
        )

pico_add_extra_outputs(SensorLib)

