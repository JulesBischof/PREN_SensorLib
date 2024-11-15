// PhotoSensor.hpp

#ifndef PHOTOSENSOR
#define PHOTOSENSOR

#include "pico/stdlib.h"
#include "PhotoSensorArray.hpp"

#include <unordered_map>

// forward declaration
class PhotoSensorArray;

class PhotoSensor
{
public:
    PhotoSensor(int gpio, PhotoSensorArray *sensorArray, uint8_t id);

    void StartMeasurement();

    void handle_interrupt();

    uint get_pin() const { return gpio; }

    volatile absolute_time_t dischargeValue;

    // static Interrupt Handler for all Sensor gpios, hence static
    static void Sensor_gpio_ISR(uint gpio, uint32_t events);

private:
    int gpio;

    // represents number of Cell inside the Sonsor-array
    uint8_t id;

    absolute_time_t timestamp;

    PhotoSensorArray *photoSensorArray;

    static int64_t ChargeCapCallback(alarm_id_t id, void *user_data);

    // global unordered Map with all the Sensor instances, mapped to their GPIO
    // essential for the ISR, which has to figure out wich Instance fired the Interrupt
    static std::unordered_map<int, PhotoSensor *> PhotoSensorCells_GPIO_Map;
};

#endif