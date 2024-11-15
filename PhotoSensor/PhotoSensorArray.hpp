// PhotoSensorArray.hpp

#ifndef PHOTOSENSORARRAY
#define PHOTOSENSORARRAY

#include "pico/stdlib.h"
#include "PhotoSensor.hpp"

#define N_SENSORS 5

class PhotoSensorArray
{
public:
    PhotoSensorArray(int gpios[N_SENSORS]);

    uint64_t *ReadValues();

    volatile bool idle;

private:
    PhotoSensor *sensors[N_SENSORS];
};

#endif