#include "TMC5240.hpp"

// ---------------
// statics & consts
// ---------------
bool TMC5240::_spi_initialized = false;
// ---------------

bool TMC5240::_spiWriteRegister(uint8_t address, uint32_t value)
{

    uint8_t tx_buffer[5] = {(address | 0x80) & 0xFF, // write - command: MSB = 1
                            (value >> 24) & 0xFF,
                            (value >> 16) & 0xFF,
                            (value >> 8) & 0xFF,
                            value & 0xFF};
    uint8_t rx_buffer[5] = {0};

    gpio_put(_csn_pin, 0); // pull down CS
    sleep_us(1);
    spi_write_read_blocking(_spi_inst, tx_buffer, rx_buffer, 5);
    gpio_put(_csn_pin, 1); // pull up CS

    // get status flags
    this->_spi_status_flags = (rx_buffer[0]);

    return true;
}

uint32_t TMC5240::_spiReadRegister(uint8_t address)
{
    uint8_t tx_buffer[5] = {address & 0x7F, 0, 0, 0, 0}; // Read command - MSB = 0
    uint8_t rx_buffer[5] = {0};

    // push address - take a look into datasheet, pipeline structure
    gpio_put(_csn_pin, 0); // pull down CS
    sleep_us(1);
    spi_write_read_blocking(_spi_inst, tx_buffer, rx_buffer, 5);
    gpio_put(_csn_pin, 1); // pull up CS

    // get actual data - take a look into datasheet, pipeline structure
    gpio_put(_csn_pin, 0); // pull down CS
    sleep_us(1);
    spi_write_read_blocking(_spi_inst, tx_buffer, rx_buffer, 5);
    gpio_put(_csn_pin, 1); // pull up CS

    this->_spi_status_flags = (rx_buffer[0]);

    // reconstruct register value
    uint32_t result = (rx_buffer[1] << 24) |
                      (rx_buffer[2] << 16) |
                      (rx_buffer[3] << 8) |
                      (rx_buffer[4]);
    return result;
}

uint32_t TMC5240::_spiReadBitField(uint8_t address, uint32_t mask, uint8_t shift)
{
    // get Register
    uint32_t reg_value = _spiReadRegister(address);
    // mask Bitfield
    uint32_t bitfield = (reg_value & mask) >> shift;
    return bitfield;
}

TMC5240::TMC5240(uint8_t rx_pin, uint8_t tx_pin, uint8_t csn_pin, uint8_t sclk_pin, spi_inst_t *spi_inst)
{
    _rx_pin = rx_pin;
    _tx_pin = tx_pin;
    _csn_pin = csn_pin;
    _sclk_pin = sclk_pin;
    _spi_inst = spi_inst;

    if (!_spi_initialized)
    {
        spi_init(_spi_inst, 1000 * SPI_BAUDRATE_KHZ);
        gpio_set_function(_rx_pin, GPIO_FUNC_SPI);
        gpio_set_function(_tx_pin, GPIO_FUNC_SPI);
        gpio_set_function(_sclk_pin, GPIO_FUNC_SPI);

        _spi_initialized = true;
    }

    gpio_init(csn_pin);
    gpio_set_dir(csn_pin, GPIO_OUT);
    gpio_put(csn_pin, 1); // pull up CS

    spi_set_format(_spi_inst, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // uint32_t version = 0;
    // version = _spiReadRegister(TMC5240_INP_OUT);
    uint8_t version = _spiReadBitField(TMC5240_INP_OUT, TMC5240_VERSION_MASK, TMC5240_VERSION_SHIFT);

    printf("TMC5240 Version %d initialized! \n", version);

    bool drvEnn = _spiReadBitField(TMC5240_INP_OUT, TMC5240_DRV_ENN_MASK, TMC5240_DRV_ENN_SHIFT);
    printf("DRV_ENN State = %d \n", drvEnn);
}

void TMC5240::initCurrentSetting()
{
    uint32_t DRV_CONF_val = _spiReadRegister(TMC5240_DRV_CONF);
    DRV_CONF_val |= CURRENT_RANGE;
    _spiWriteRegister(TMC5240_DRV_CONF, DRV_CONF_val);

    _spiWriteRegister(TMC5240_GLOBAL_SCALER, GLOBSCALER);

    uint32_t IHOLD_IRUN_val = _spiReadRegister(TMC5240_IHOLD_IRUN);
    IHOLD_IRUN_val |= IHOLD << TMC5240_IHOLD_SHIFT;
    IHOLD_IRUN_val |= IRUN << TMC5240_IRUN_SHIFT;
    IHOLD_IRUN_val |= IHOLDDELAY << TMC5240_IHOLDDELAY_SHIFT;
    IHOLD_IRUN_val |= IRUNDELAY << TMC5240_IRUNDELAY_SHIFT;
    _spiWriteRegister(TMC5240_IHOLD_IRUN, IHOLD_IRUN_val);

    _spiWriteRegister(TMC5240_TPOWERDOWN, TPOWERDOWN);

    uint32_t drv_conf_val = _spiReadRegister(TMC5240_DRV_CONF);
    uint32_t globscalerval = _spiReadRegister(TMC5240_GLOBAL_SCALER);
    uint32_t ihold_irun_val = _spiReadRegister(TMC5240_IHOLD_IRUN);

    printf("Current Settings Register Values \n --- GLOBSCALER ... 0x%x \n --- DRV_CONF ... 0x%x \n --- IHOLD_IRUN ... %x \n", globscalerval, drv_conf_val, ihold_irun_val);
}

void TMC5240::initSpreadCycle()
{
    uint32_t GCONF_val = _spiReadRegister(TMC5240_GCONF);
    GCONF_val |= GCONF << TMC5240_EN_PWM_MODE_SHIFT;
    _spiWriteRegister(TMC5240_GCONF, GCONF_val);

    uint32_t CHOPCONF_val = _spiReadRegister(TMC5240_CHOPCONF);
    CHOPCONF_val |= TOFF << TMC5240_TOFF_SHIFT;
    CHOPCONF_val |= HSTRT << TMC5240_SMALL_HYSTERESIS_SHIFT;
    CHOPCONF_val |= HEND << TMC5240_HEND_OFFSET_SHIFT;
    _spiWriteRegister(TMC5240_CHOPCONF, CHOPCONF_val);

    return;
}

void TMC5240::setShaftDirection(bool direction)
{
    uint32_t GCONF_val = _spiReadRegister(TMC5240_GCONF);
    GCONF_val |= direction << TMC5240_SHAFT_SHIFT;
    _spiWriteRegister(TMC5240_GCONF, GCONF_val);

    return;
}

/// @brief
/// @param direction true if going backwarts
/// @param vmax velocity in [uSteps/t]
/// @param amax acceleration in [uSteps/ta^2]
void TMC5240::moveVelocityMode(bool direction, uint32_t vmax, uint32_t amax)
{
    _spiWriteRegister(TMC5240_RAMPMODE, direction ? TMC5240_MODE_VELPOS : TMC5240_MODE_VELNEG);

    _spiWriteRegister(TMC5240_VMAX, vmax);
    _spiWriteRegister(TMC5240_AMAX, amax);

    uint32_t vmax_val = _spiReadRegister(TMC5240_VMAX);
    uint32_t amax_val = _spiReadRegister(TMC5240_AMAX);
    printf("vmax set to: %d ; amax set to: %d \n", vmax_val, amax_val);

    return;
}
