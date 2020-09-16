#pragma once

// ALS31300 magnetometer driver

/*
 * Note: this device returns 32-bit register values in MSB order
 */

// there is no 0x01 register
constexpr auto ALS31300_CONF_REG = 0x02;
typedef struct
{
    uint8_t user_eeprom : 5;
    bool int_latch_enable : 1;
    bool channel_X_en : 1;
    bool channel_Y_en : 1;
    bool channel_Z_en : 1;
    bool I2C_threshold : 1;
    bool slave_addr : 7;
    bool disable_slave_ADC : 1;
    bool I2C_CRC_en : 1;
    uint8_t hall_mode : 2;
    uint8_t bandwidth : 3;
} als31300_conf_reg;

// --------
constexpr auto ALS31300_INT_REG = 0x03;

typedef struct
{
    uint8_t int_X_threshold : 6;
    uint8_t int_Y_threshold : 6;
    uint8_t int_Z_threshold : 6;
    bool int_X_en : 1;
    bool int_Y_en : 1;
    bool int_Z_en : 1;
    bool int_eeprom_en : 1;
    bool int_eeprom_status : 1;
    bool int_mode : 1;
    bool int_signed_en : 1;

} als31300_int_reg;

// --------
constexpr auto ALS31300_PWR_REG = 0x27;

typedef struct
{
    uint8_t sleep : 2;
    uint8_t I2C_loop_mode : 2;
    uint8_t count_max_LP_mode : 3;
} als31300_pwr_reg;

// --------
constexpr auto ALS31300_MEASUREMENTS_MSB_REG = 0x28;

typedef struct
{
    uint8_t temperature_MSB : 6;
    bool int_flag : 1;
    bool new_data_flag : 1;
    uint8_t Z_MSB : 8;
    uint8_t Y_MSB : 8;
    uint8_t X_MSB : 8;
} als31300_measurements_MSB_reg;

// --------
constexpr auto ALS31300_MEASUREMENTS_LSB_REG = 0x29;

typedef struct
{
    uint8_t temperature_LSB : 6;
    bool hall_mode_status : 2;
    uint8_t Z_LSB : 4;
    uint8_t Y_LSB : 4;
    uint8_t X_LSB : 4;
    bool int_eeprom_write_pending : 1;
} als31300_measurements_LSB_reg;

float als31300_temperature_convert( uint16_t temperature_12bit)
{
    const int32_t intermediate = temperature_12bit - 1708;
    return intermediate * 0.07373046875;
}
