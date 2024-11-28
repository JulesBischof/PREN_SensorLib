add_library( Tmc5240
        Tmc5240.hpp
        Tmc5240.cpp
        TMC5240_HW_Abstraction.h
        )


# Add the standard library to the build
target_link_libraries(Tmc5240
        pico_stdlib)

# include libraries
target_link_libraries(Tmc5240
        hardware_timer
        hardware_spi
        )

target_include_directories(TFLuna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")