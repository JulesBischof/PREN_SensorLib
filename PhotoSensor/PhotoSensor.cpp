#include "PhotoSensor.hpp"
#include <stdio.h>

#include "time.h"

// definition static variables
int PhotoSensor::n_sensors = 0;
PhotoSensor *PhotoSensor::instances[5];

/// @brief Single Sensor Cell
/// @param gpio Gpio the Sensor is connected to
PhotoSensor::PhotoSensor(int gpio)
{
    this->gpio = gpio;

    // save sensor to instances List
    instances[n_sensors++] = this;

    // init gpio and set high as default value
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true); // High state

    idle = true;
    dischargeValue = 0;

    // Register isr callback
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
}

/// @brief Starts the Measurment on one of the Instances
void PhotoSensor::StartMeasurement()
{
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true);
    idle = false;

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
    idle = true;

    // dissable irq
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
}

/// @brief Detects, wich Instance fired the Interrupt and jumps into it's ISR
/// @param gpio GPIO the Sensor Cell is connected to
/// @param events argument gor the ISR
void PhotoSensor::gpio_callback(uint gpio, uint32_t events)
{
    for (int i = 0; i < n_sensors; i++)
    {
        if (instances[i] != nullptr && instances[i]->get_pin() == gpio)
        {
            instances[i]->handle_interrupt(); // trigger object isr
        }
    }
}
