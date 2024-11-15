// PhotoSensor.hpp

#ifndef PHOTOSENSOR
#define PHOTOSENSOR

#include "pico/stdlib.h"

class PhotoSensor
{
public:
    PhotoSensor(int gpio);

    void StartMeasurement();

    void handle_interrupt();

    static void gpio_callback(uint gpio, uint32_t events);

    // Sensor Array and counter
    static PhotoSensor *instances[5];
    static int n_sensors;

    uint get_pin() const { return gpio; }

    volatile bool idle;

    absolute_time_t dischargeValue;

private:
    int gpio;
    absolute_time_t timestamp;

    static int64_t ChargeCapCallback(alarm_id_t id, void *user_data);
};

#endif