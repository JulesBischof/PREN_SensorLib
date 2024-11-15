// PhotoSensorArray.hpp

#ifndef PHOTOSENSORARRAY
#define PHOTOSENSORARRAY

#include "pico/stdlib.h"
#include "PhotoSensor.hpp"

#include <map>

#define N_SENSORS 5
#define IDLE_STATE 0x1F

// forward - declaration
class PhotoSensor;

class PhotoSensorArray
{
public:
    PhotoSensorArray(int gpios[N_SENSORS]);

    uint64_t *ReadValues();

    // 8 bits representing idle sate of each sensor
    volatile char idleState;

    std::map<int, PhotoSensor *> SensorInstances;

private:
};

#endif