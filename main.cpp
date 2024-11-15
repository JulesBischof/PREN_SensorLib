#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "PhotoSensor/PhotoSensorArray.hpp"

int main()
{
    stdio_init_all();
    alarm_pool_init_default();

    int gpios[5] = {10, 11, 12, 13, 14};

    bool boolVals[5];

    PhotoSensorArray SensorArray(gpios);

    while (true)
    {
        uint64_t *values = SensorArray.ReadValues();

        printf("raw...  1| %lld |   2| %lld |   3| %lld |   4| %lld |   5| %lld |\n", values[0], values[1], values[2], values[3], values[4]);

        delete values;
    }
}