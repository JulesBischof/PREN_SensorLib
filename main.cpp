#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "PhotoSensor/PhotoSensorArray.hpp"
#include "MPU6050/MPU6050.hpp"

#include "TMC5240/TMC5240.hpp"

#include "TFLuna/TFLuna.hpp"

int main()
{
    stdio_init_all();
    // alarm_pool_init_default();

    TMC5240 stepper(16, 19, 17, 18, spi0);

    stepper.initCurrentSetting();
    stepper.initSpreadCycle();

    stepper.moveVelocityMode(0, 200000, 5000);

    sleep_ms(2000);

    stepper.moveVelocityMode(0, 400000, 5000);

    sleep_ms(2000);

    stepper.moveVelocityMode(0, 3000000, 100000);

    sleep_ms(5000);

    stepper.moveVelocityMode(0, 0, 1000);
}

// Gyro Test ===========================================

// i2c_init(i2c0, 100 * 1000); // baudrate = 100kHz

// printf("GyroTest\n");

// Mpu6050 gyro(17, 16, 18, i2c0, MPU6050_ADDRESS_AD0_LOW);

// while (true)
// {
//     printf("angle: ------ %.2f \n", gyro.getAngleZ());
// }

// Liniensensor Test ===========================================

// int gpios[5] = {10, 11, 12, 13, 14};

// bool boolVals[5];

// PhotoSensorArray SensorArray(gpios);

// while (true)
// {
//     uint64_t *values = SensorArray.ReadValues();

//     printf("raw...  1| %lld |   2| %lld |   3| %lld |   4| %lld |   5| %lld |\n", values[0], values[1], values[2], values[3], values[4]);

//     delete values;
// }

// LIDAR Test =====================================================

// i2c_init(i2c0, 100 * 1000); // baudrate = 100kHz

// TFLuna lidar(17, 16, 18, i2c0, TFL_DEF_ADR);

// while (true)
// {
//     int16_t dist = 0;
//     int16_t flux = 0;
//     int16_t temp = 0;

//     lidar.getData(dist, flux, temp);

//     printf("dist = %d, flux = %d, temp = %d \n", dist, flux, temp);

//     sleep_ms(1000);
// }