#ifndef MPU6050
#define MPU6050

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"

class Mpu6050
{
public:
    Mpu6050(uint8_t scl_pin, uint8_t sda_pin, uint8_t int_pin, i2c_inst_t *i2c, uint8_t i2c_addr);

    float getAngleZ();
    void setAngleZ(float val);
    int16_t readAngularSpeedZ();

    uint16_t mpu6050_GetFIFOAvg();
    uint16_t mpu6050_PopFIFO();

    volatile float bias;

private:
    int mpu650init();

    volatile float _angle_z;

    void _i2c_write_register(uint8_t reg, uint8_t data);
    uint8_t _i2c_read_register(uint8_t reg);
    void _i2c_read_registers(uint8_t reg, uint8_t *buffer, size_t length);

    uint16_t mpu6050_GetFIFOCount();

    uint8_t _scl_pin, _sda_pin, _int_pin;
    i2c_inst_t *_i2c;
    uint8_t _i2c_addr;
};

#endif // end MPU6050