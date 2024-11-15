#include "PhotoSensor.hpp"
#include <stdio.h>

#include "time.h"

// definition static unordered map
std::unordered_map<int, PhotoSensor *> PhotoSensor::PhotoSensorCells_GPIO_Map;

/// @brief Single Sensor Cell
/// @param gpio Gpio the Sensor is connected to
/// @param photoSensorArray Instance of the Sensor Array
/// @param id unique id the SensorCell remembers it's physical place in the Array
PhotoSensor::PhotoSensor(int gpio, PhotoSensorArray *photoSensorArray, uint8_t id)
{
    this->gpio = gpio;
    this->photoSensorArray = photoSensorArray;
    this->id = id;

    // unordered map -> needed for gpio-instances lookuptable for ISR gpio_callback
    PhotoSensorCells_GPIO_Map[gpio] = this;

    // init gpio and set high as default value
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true); // High state

    photoSensorArray->idleState = photoSensorArray->idleState | 1 << id;

    dischargeValue = 0;

    // Register isr callback
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL, true, Sensor_gpio_ISR);
}

/// @brief Starts the Measurment on one of the Instances
void PhotoSensor::StartMeasurement()
{
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true);

    photoSensorArray->idleState = photoSensorArray->idleState | 0 << id;

    sleep_ms(50);

    timestamp = get_absolute_time();
    gpio_set_pulls(gpio, false, false); // open drain
    gpio_set_dir(gpio, GPIO_IN);        // set to input

    // enable irq
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
}

/// @brief Timer Callback after capacitor got charged
/// @param id alarm ID
/// @param user_data Reference to SensorInstance, who's Capacitor got charged
/// @return 0
int64_t PhotoSensor::ChargeCapCallback(alarm_id_t id, void *user_data)
{
    PhotoSensor *sensor = (PhotoSensor *)user_data;
    sensor->timestamp = get_absolute_time();
    gpio_set_dir(sensor->get_pin(), GPIO_IN); // Make sure it's in input mode
    return 0;
}

/// @brief ISR for the single Sensor Instance
void PhotoSensor::handle_interrupt()
{

    absolute_time_t time = get_absolute_time();
    dischargeValue = absolute_time_diff_us(timestamp, time);
    // printf("Interrupt fired! Value: %d\n", dischargeValue);

    photoSensorArray->idleState = photoSensorArray->idleState | 1 << id;

    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
}

/// @brief static function! Detects, wich Instance fired the Interrupt and jumps into it's ISR
/// @param gpio GPIO the Sensor Cell is connected to
/// @param events argument given by the ISR
void PhotoSensor::Sensor_gpio_ISR(uint gpio, uint32_t events)
{
    PhotoSensorCells_GPIO_Map[gpio]->handle_interrupt(); // trigger object isr
    return;
}
