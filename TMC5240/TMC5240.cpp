#include "TMC5240.hpp"

bool TMC5240::_spiWriteRegister(uint8_t address, uint32_t value)
{
    uint8_t buffer[5] = {0};

    buffer[0] = address | 0x80; // MSB = 1 -> start write
    buffer[1] = (value >> 24) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 8) & 0xFF;
    buffer[4] = value & 0xFF;

    gpio_put(_csn_pin, false); // pull down CS
    spi_write_blocking(_spi_inst, buffer, 5);
    gpio_put(_csn_pin, true); // pull up CS

    return true;
}

uint32_t TMC5240::_spiReadRegister(uint8_t address)
{
    uint8_t tx_buffer[5] = {address & 0x7F, 0, 0, 0, 0}; // Write commande - MSB = 1
    uint8_t rx_buffer[5] = {0};

    gpio_put(_csn_pin, 0); // pull down CS
    spi_write_read_blocking(_spi_inst, tx_buffer, rx_buffer, 5);
    gpio_put(_csn_pin, 1); // pull up CS

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

    uint8_t version = _spiReadBitField(TMC5240_INP_OUT, TMC5240_VERSION_MASK, TMC5240_VERSION_SHIFT);

    printf("TMC5240 Version %d initialized!", version);
}
