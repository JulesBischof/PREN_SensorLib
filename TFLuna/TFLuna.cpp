/*
 * original Developer: Bud Ryerson reworked by Julian Bischof for RP2040
 *  orig Repo ...
 *  https://github.com/budryerson/TFLuna-I2C/blob/master/
 */

#include <TFLuna.hpp>

// Constructor/Destructor
TFLuna::TFLuna(uint8_t scl_pin, uint8_t sda_pin, uint8_t int_pin, i2c_inst_t *i2c, uint8_t i2c_addr)
{
    this->_scl_pin = scl_pin;
    this->_sda_pin = sda_pin;
    this->_int_pin = int_pin;
    this->_i2c = i2c;

    this->_i2c_addr = i2c_addr;

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    this->tfStatus = 0;
}

TFLuna::~TFLuna() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - -
//             GET DATA FROM THE DEVICE
// - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TFLuna::getData(int16_t &dist, int16_t &flux, int16_t &temp)
{
    tfStatus = TFL_READY; // clear status of any error condition

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 1 - Use the `Wire` function `readReg` to fill the six byte
    // `dataArray` from the contiguous sequence of registers `TFL_DIST_LO`
    // to `TFL_TEMP_HI` that declared in the header file 'TFLuna.h`.
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    uint8_t buff[6] = {0};

    for (uint8_t i = TFL_DIST_LO; i <= TFL_TEMP_HI; i++)
    {
        if (!readReg(i, &buff[i - TFL_DIST_LO]))
            return false;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Step 2 - Shift data from read array into the three variables
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    dist = (buff[0] + (buff[1] << 8));
    flux = (buff[2] + (buff[3] << 8));
    temp = (buff[4] + (buff[5] << 8));

    /*
        // Convert temperature from hundredths
        // of a degree to a whole number
        temp = int16_t( temp / 100);
        // Then convert Celsius to degrees Fahrenheit
        temp = uint8_t( temp * 9 / 5) + 32;
    */

    // - - Evaluate Abnormal Data Values - -
    // Signal strength <= 100
    if (flux < (int16_t)100)
    {
        tfStatus = TFL_WEAK;
        return false;
    }
    // Signal Strength saturation
    else if (flux == (int16_t)0xFFFF)
    {
        tfStatus = TFL_STRONG;
        return false;
    }
    else
    {
        tfStatus = TFL_READY;
        return true;
    }
}

// Get Data short version
bool TFLuna::getData(int16_t &dist)
{
    static int16_t flux, temp;
    return getData(dist, flux, temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - -
//              EXPLICIT COMMANDS
// - - - - - - - - - - - - - - - - - - - - - - - - - -

//  = =  GET DEVICE TIME (in milliseconds) = = =
//  Pass back time as an unsigned 16-bit variable
uint16_t TFLuna::Get_Time()
{
    uint8_t buff = 0;

    uint16_t retVal = 0;

    if (!readReg(TFL_TICK_LO, &buff))
        return 0;
    else
        retVal = buff; // Read into `tim` array
    if (!readReg(TFL_TICK_HI, &buff))
        return 0;
    else
        retVal |= buff << 8;
    return retVal;
}

//  = =  GET PRODUCTION CODE (Serial Number) = = =
// When you pass an array as a parameter to a function
// it decays into a pointer to the first element of the array.
// The 14 byte array variable `tfCode` declared in the example
// sketch decays to the array pointer `p_cod`.
bool TFLuna::Get_Prod_Code(uint8_t *p_cod)
{
    for (uint8_t i = 0; i < 14; ++i)
    {
        uint8_t buff = 0;
        if (!readReg((0x10 + i), &buff))
            return false;
        else
            p_cod[i] = buff; // Read into product code array
    }
    return true;
}

//  = = = =    GET FIRMWARE VERSION   = = = =
// The 3 byte array variable `tfVer` declared in the
// example sketch decays to the array pointer `p_ver`.
bool TFLuna::Get_Firmware_Version(uint8_t *p_ver)
{
    for (uint8_t i = 0; i < 3; ++i)
    {
        uint8_t buff = 0;
        if (!readReg((0x0A + i), &buff))
            return false;
        else
            p_ver[i] = buff;
    }
    return true;
}

//  = = = = =    SAVE SETTINGS   = = = = =
bool TFLuna::Save_Settings()
{
    return (writeReg(TFL_SAVE_SETTINGS, 0x01));
}

//  = = = =   SOFT (SYSTEM) RESET   = = = =
bool TFLuna::Soft_Reset()
{
    return (writeReg(TFL_SOFT_RESET, 0x02));
}

//  = = = = = =    SET I2C ADDRESS   = = = = = =
// Range: 0x08, 0x77. Must reboot to take effect.
bool TFLuna::Set_I2C_Addr(uint8_t adrNew)
{
    return (writeReg(TFL_SET_I2C_ADDR, adrNew));
}

//  = = = = =   SET ENABLE   = = = = =
bool TFLuna::Set_Enable()
{
    return (writeReg(TFL_DISABLE, 0x01));
}

//  = = = = =   SET DISABLE   = = = = =
bool TFLuna::Set_Disable()
{
    return (writeReg(TFL_DISABLE, 0x00));
}

//  = = = = = =    SET FRAME RATE   = = = = = =
bool TFLuna::Set_Frame_Rate(uint16_t frm)
{
    uint8_t buff[2] = {frm << 8,
                       frm >> 8};

    if (!writeReg(TFL_FPS_LO, buff[0]))
        return false;
    if (!writeReg(TFL_FPS_HI, buff[1]))
        return false;
    return true;
}

//  = = = = = =    GET FRAME RATE   = = = = = =
uint16_t TFLuna::Get_Frame_Rate()
{
    uint8_t buff[2] = {0};

    if (!readReg(TFL_FPS_LO, &buff[0]))
        return false;
    if (!readReg(TFL_FPS_HI, &buff[1]))
        return false;

    return (uint16_t)(buff[0] << 8 | buff[1]);
    return true;
}

//  = = = =   HARD RESET to Factory Defaults  = = = =
bool TFLuna::Hard_Reset()
{
    return (writeReg(TFL_HARD_RESET, 0x01));
}

//  = = = = = =   SET CONTINUOUS MODE   = = = = = =
// Sample LiDAR chip continuously at Frame Rate
bool TFLuna::Set_Cont_Mode()
{
    return (writeReg(TFL_SET_TRIG_MODE, 0x00));
}

//  = = = = = =   SET TRIGGER MODE   = = = = = =
// Device will sample only once when triggered
bool TFLuna::Set_Trig_Mode()
{
    return (writeReg(TFL_SET_TRIG_MODE, 0x01));
}

//  = = = = = =   SET TRIGGER   = = = = = =
// Trigger device to sample once
bool TFLuna::Set_Trigger()
{
    return (writeReg(TFL_TRIGGER, 0x01));
}
//
// = = = = = = = = = = = = = = = = = = = = = = = =

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//       READ OR WRITE A GIVEN REGISTER OF THE SLAVE DEVICE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TFLuna::readReg(uint8_t reg, uint8_t *buffer)
{
    int err = 0;

    err = i2c_write_timeout_us(_i2c, _i2c_addr, &reg, 1, true, 10000);
    err = i2c_read_timeout_us(_i2c, _i2c_addr, buffer, 1, false, 10000);

    if (err != 1)
    {
        tfStatus = err; // errors = PICO_ERROR_GENERIC or PICO_ERROR_TIMEOUT
        return false;
    }

    return true;
}

bool TFLuna::writeReg(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2] = {reg, data};

    int err = 0;

    err = i2c_write_timeout_us(_i2c, _i2c_addr, buffer, 2, true, 10000);

    if (err != 2) // 2 due to data send
    {
        tfStatus = err; // errors = PICO_ERROR_GENERIC or PICO_ERROR_TIMEOUT
        return false;
    }

    return true;
}