#include "PhotoSensorArray.hpp"

PhotoSensorArray::PhotoSensorArray(int gpios[N_SENSORS])
{
    for (int i = 0; i < N_SENSORS; i++)
    {
        sensors[i] = new PhotoSensor(gpios[i]);
    }
}

uint64_t *PhotoSensorArray::ReadValues()
{
    uint64_t *retval = new uint64_t[N_SENSORS];

    for (int i = 0; i < N_SENSORS; i++)
    {
        sensors[i]->StartMeasurement();
    }

    for (int i = 0; i < N_SENSORS; i++)
    {
        while (!sensors[i]->idle)
        {
            tight_loop_contents();
        }
        retval[i] = sensors[i]->dischargeValue;
    }

    return retval;
}
