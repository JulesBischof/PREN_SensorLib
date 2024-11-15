#include "PhotoSensorArray.hpp"

PhotoSensorArray::PhotoSensorArray(int gpios[N_SENSORS])
{
    idleState = 0;

    for (int i = 0; i < N_SENSORS; i++)
    {
        SensorInstances[i] = new PhotoSensor(gpios[i], this, i);
    }
}

uint64_t *PhotoSensorArray::ReadValues()
{
    uint64_t *retval = new uint64_t[N_SENSORS];

    idleState = 0;

    for (int i = 0; i < N_SENSORS; i++)
    {
        SensorInstances[i]->StartMeasurement();
    }

    while (idleState != IDLE_STATE)
    {
    } // wait until all cells are read

    for (int i = 0; i < N_SENSORS; i++)
    {
        retval[i] = SensorInstances[i]->dischargeValue;
    }
    return retval;
}
