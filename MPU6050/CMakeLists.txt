add_library( MPU6050
        MPU6050.hpp
        MPU6050.cpp
        mpu6050.h
        )


# Add the standard library to the build
target_link_libraries(MPU6050
        pico_stdlib)

# include libraries
target_link_libraries(MPU6050
        hardware_timer
        hardware_i2c
        )

target_include_directories(MPU6050 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")