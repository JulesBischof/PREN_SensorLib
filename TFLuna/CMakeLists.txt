add_library( TFLuna
        TFLuna.hpp
        TFLuna.cpp
        )


# Add the standard library to the build
target_link_libraries(TFLuna
        pico_stdlib)

# include libraries
target_link_libraries(TFLuna
        hardware_timer
        hardware_i2c
        )

target_include_directories(TFLuna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")