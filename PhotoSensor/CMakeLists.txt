add_library( PhotoSensorArray
                PhotoSensorArray.hpp
                PhotoSensorArray.cpp
                PhotoSensor.hpp
                PhotoSensor.cpp
            )


# Add the standard library to the build
target_link_libraries(PhotoSensorArray
        pico_stdlib)

# include libraries
target_link_libraries(PhotoSensorArray
        hardware_timer
        )

target_include_directories(PhotoSensorArray PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")