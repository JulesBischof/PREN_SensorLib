#include "MPU6050.hpp"
#include "mpu6050.h"

#define SENSITIVITY_FACTOR 131 // @ gyro config 250°/sek FS_SEL=0
#define RAW_THRESHHOLD 20
#define TIMER_DIFF_MS 50

static bool timer_readZVal_callback(struct repeating_timer *t)
{
    Mpu6050 *instance = (Mpu6050 *)t->user_data;

    float vel = (instance->readAngularSpeedZ()) - instance->bias;

    if (vel < RAW_THRESHHOLD && vel > -RAW_THRESHHOLD)
        return true;

    vel = vel / (SENSITIVITY_FACTOR); // calc unit °/s

    float angularchange = vel * ( (float)TIMER_DIFF_MS / 1000); // calc new angle

    float angle = instance->getAngleZ();

    angle += angularchange;

    instance->setAngleZ(angle);

    return true; // restart timer
}

Mpu6050::Mpu6050(uint8_t scl_pin, uint8_t sda_pin, uint8_t int_pin, i2c_inst_t *i2c, uint8_t i2c_addr)
{
    this->_scl_pin = scl_pin;
    this->_sda_pin = sda_pin;
    this->_int_pin = int_pin;
    this->_i2c = i2c;
    this->_i2c_addr = i2c_addr;

    _angle_z = 0;

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    mpu650init();

    sleep_ms(100);

    uint8_t msg = MPU6050_RA_WHO_AM_I, res = 0;
    i2c_write_blocking(_i2c, _i2c_addr, &msg, 1, true);
    i2c_read_blocking(_i2c, _i2c_addr, &res, 1, false);

    printf("Connection to Gyro MPU6050 Address: 0x%x build up\n", res);

    static struct repeating_timer timer;
    add_repeating_timer_ms(TIMER_DIFF_MS, timer_readZVal_callback, this, &timer);
}

float Mpu6050::getAngleZ()
{
    return this->_angle_z;
}

void Mpu6050::setAngleZ(float val)
{
    this->_angle_z = val;
}

int16_t Mpu6050::readAngularSpeedZ()
{
    int err = 0;
    uint8_t addr = 0;
    uint8_t resL = 0;
    uint8_t resH = 0;

    addr = MPU6050_RA_GYRO_ZOUT_L;
    err = i2c_write_blocking(_i2c, _i2c_addr, &addr, 1, true);
    err = i2c_read_blocking(_i2c, _i2c_addr, &resL, 1, false);

    addr = MPU6050_RA_GYRO_ZOUT_H;
    err = i2c_write_blocking(_i2c, _i2c_addr, &addr, 1, true);
    err = i2c_read_blocking(_i2c, _i2c_addr, &resH, 1, false);

    return (int16_t)((resH << 8) | resL);
}

void Mpu6050::_i2c_write_register(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2] = {reg, data};
    i2c_write_blocking(_i2c, _i2c_addr, buffer, 2, false);
}

uint8_t Mpu6050::_i2c_read_register(uint8_t reg)
{
    uint8_t value;
    i2c_write_blocking(_i2c, _i2c_addr, &reg, 1, true);
    i2c_read_blocking(_i2c, _i2c_addr, &value, 1, false);
    return value;
}

void Mpu6050::_i2c_read_registers(uint8_t reg, uint8_t *buffer, size_t length)
{
    i2c_write_blocking(_i2c, _i2c_addr, &reg, 1, true);
    i2c_read_blocking(_i2c, _i2c_addr, buffer, length, false);
}

uint16_t Mpu6050::mpu6050_PopFIFO()
{
    uint8_t buffer[2];
    _i2c_read_registers(MPU6050_RA_FIFO_R_W, buffer, 2);
    return (buffer[0] << 8) | buffer[1];
}

uint16_t Mpu6050::mpu6050_GetFIFOCount()
{
    uint8_t buffer[2];
    _i2c_read_registers(MPU6050_RA_FIFO_COUNTH, buffer, 2);
    return (buffer[0] << 8) | buffer[1];
}

uint16_t Mpu6050::mpu6050_GetFIFOAvg() // FUNKTIONIERT NICHT
{
    uint16_t fifoSize = mpu6050_GetFIFOCount();

    if (fifoSize < 2 || fifoSize % 2 != 0)
    {
        // FIFO musst be at least 2 Bytes long
        return 0;
    }

    uint8_t *buffer = new uint8_t[fifoSize];
    _i2c_read_registers(MPU6050_RA_FIFO_R_W, buffer, fifoSize); // FIFO-Data read

    uint32_t sum = 0;
    for (int i = 0; i < fifoSize; i += 2)
    {
        // combine 2 Bytes to a 16-bit val
        uint16_t value = (buffer[i] << 8) | buffer[i + 1];
        sum += value;
    }

    delete[] buffer; // free memory

    _i2c_write_register(MPU6050_RA_USER_CTRL, 0x04); // FIFO reset

    return sum / (fifoSize / 2); // calc average
}

int Mpu6050::mpu650init()
{
    // reset MPU6050
    // _i2c_write_register(MPU6050_RA_PWR_MGMT_1, 1 << MPU6050_PWR1_DEVICE_RESET_BIT);
    // sleep_ms(300);

    // Wake up the MPU6050
    _i2c_write_register(MPU6050_RA_PWR_MGMT_1, 0x00);

    // Set DLPF (Digital Low Pass Filter) to 42 Hz
    _i2c_write_register(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_42);

    // Set the sample rate to 1 kHz (divider = 0)
    _i2c_write_register(MPU6050_RA_SMPLRT_DIV, 0x00);

    // Configure the Gyro to ±250°/s
    _i2c_write_register(MPU6050_RA_GYRO_CONFIG, 0x00);

    // FIFO enable
    _i2c_write_register(MPU6050_RA_USER_CTRL, 0x40); // FIFO_EN aktivieren

    // Enable FIFO just for Z-gyro
    _i2c_write_register(MPU6050_RA_FIFO_EN, 1 << MPU6050_ZG_FIFO_EN_BIT);

    // Enable FIFO overflow interrupt
    _i2c_write_register(MPU6050_RA_INT_ENABLE, 0x10);

    // FIFO reset
    _i2c_write_register(MPU6050_RA_USER_CTRL, 1 << MPU6050_USERCTRL_FIFO_RESET_BIT);

    sleep_ms(100); //wait until first measurment

    // get bias
    bias = (float)readAngularSpeedZ();

    return 0;
}