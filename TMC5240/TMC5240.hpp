#ifndef TMC5240_H_
#define TMC5240_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "TMC5240_HW_Abstraction.h"
#include "MotorSettings.h"

// ---------------
// Settings
// ---------------
#define SPI_BAUDRATE_KHZ 1000
// ---------------

// ---------------
// Status Flags
// ---------------
#define NOERROR 0
#define TIMEOUT 1
// ---------------

class TMC5240
{
private:
    uint8_t _rx_pin, _tx_pin, _csn_pin, _sclk_pin;
    spi_inst_t *_spi_inst;
    static bool _spi_initialized;

    uint8_t _status_f;
    uint8_t _spi_status_flags;

    bool _spiWriteRegister(uint8_t address, uint32_t value);
    uint32_t _spiReadRegister(uint8_t address);

    uint32_t _spiReadBitField(uint8_t address, uint32_t mask, uint8_t shift);

public:
    TMC5240(uint8_t rx_pin, uint8_t tx_pin, uint8_t csn_pin, uint8_t sclk_pin, spi_inst_t *spi_inst);
    ~TMC5240() {};

    uint8_t getStatusFlag() { return this->_spi_status_flags; };

    void initCurrentSetting();
    void initSpreadCycle();

    void setShaftDirection(bool direction);

    void moveVelocityMode(bool direction, uint32_t vmax, uint32_t amax);
    void moveTargetMode(int32_t target_position, uint32_t v1, uint32_t v2, uint32_t a1, uint32_t a2, uint32_t dmax);
    void toggleToff();
};

#endif