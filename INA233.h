#ifndef INA233_I2C_H

#define INA233_I2C_H

#include <Arduino.h>
#include <Wire.h>

//defines a mapping of address pins A0, A1 states to I2C address. see table 6-2
enum class ADDRESS_PIN : uint8_t {
    GND     = 0b00,
    VS      = 0b01,
    SDA     = 0b10,
    SCL     = 0b11
};

//contains all possible TI-defined 7-bit I2C addresses. see table 6-2
namespace ADDRESS {
    /**
     * Recall the four possible I2C signals: GND, VS, SDA, SCL
     * 
     * There are two address pins A0, A1 on the INA233, so 2^4 = 16
     * possible combinations for I2C addresses.
     * 
     * They start at 0x40 = 0b0100 0000
     */
    inline constexpr uint8_t MAX_I2C_ADDR        = 0x77;
    inline constexpr uint8_t MIN_I2C_ADDR        = 0x08;
    inline constexpr uint8_t MASK_BASE           = 0x40;

    //address is 0b0100[xx][yy], where xx comes from A1 and yy comes from A0
    static constexpr uint8_t MASK_A0(ADDRESS_PIN mask){ return (uint8_t) mask << 0; }
    static constexpr uint8_t MASK_A1(ADDRESS_PIN mask){ return (uint8_t) mask << 2; }

    static constexpr uint8_t TARGET_ADDRESS(ADDRESS_PIN A0, ADDRESS_PIN A1){
        return MASK_BASE | MASK_A1(A1) | MASK_A0(A0);
    }
}   //namespace ADDRESS

//contains all PMBus command codes. see table 6-4
namespace COMMAND {
    inline constexpr uint8_t CLEAR_FAULTS        = 0x03;
    inline constexpr uint8_t RESTORE_DEFAULT_ALL = 0x12;
    inline constexpr uint8_t CAPABILITY          = 0x19;

    inline constexpr uint8_t IOUT_OC_WARN_LIMIT  = 0x4A;
    inline constexpr uint8_t VIN_OV_WARN_LIMIT   = 0x57;
    inline constexpr uint8_t VIN_UV_WARN_LIMIT   = 0x58;
    inline constexpr uint8_t PIN_OP_WARN_LIMIT   = 0x6B;

    inline constexpr uint8_t STATUS_BYTE         = 0x78;
    inline constexpr uint8_t STATUS_WORD         = 0x79;
    inline constexpr uint8_t STATUS_IOUT         = 0x7B;
    inline constexpr uint8_t STATUS_INPUT        = 0x7C;
    inline constexpr uint8_t STATUS_CML          = 0x7E;
    inline constexpr uint8_t STATUS_MFR_SPECIFIC = 0x80;

    inline constexpr uint8_t READ_EIN            = 0x86;
    inline constexpr uint8_t READ_VIN            = 0x88;
    inline constexpr uint8_t READ_IIN            = 0x89;
    inline constexpr uint8_t READ_VOUT           = 0x8B;
    inline constexpr uint8_t READ_IOUT           = 0x8C;

    inline constexpr uint8_t READ_POUT           = 0x96;
    inline constexpr uint8_t READ_PIN            = 0x97;

    inline constexpr uint8_t MFR_ID              = 0x99;
    inline constexpr uint8_t MFR_MODEL           = 0x9A;
    inline constexpr uint8_t MFR_REVISION        = 0x9B;
    inline constexpr uint8_t MFR_ADC_CONFIG      = 0xD0;
    inline constexpr uint8_t MFR_READ_VSHUNT     = 0xD1;
    inline constexpr uint8_t MFR_ALERT_MASK      = 0xD2;
    inline constexpr uint8_t MFR_CALIBRATION     = 0xD4;
    inline constexpr uint8_t MFR_DEVICE_CONFIG   = 0xD5;
    inline constexpr uint8_t CLEAR_EIN           = 0xD6;

    inline constexpr uint8_t TI_MFR_ID           = 0xE0;
    inline constexpr uint8_t TI_MFR_MODEL        = 0xE1;
    inline constexpr uint8_t TI_MFR_REVISION     = 0xE2;
}   //namespace COMMAND

//contains all calibration-related macros/constants. see section 6.5.2
namespace CALIBRATION {
    inline constexpr float BUSVOLTAGE_LSB        = 0.00125f;    //1.25 mV/bit
    inline constexpr float SHUNTVOLTAGE_LSB      = 0.0000025f;  //2.5 uV/bit

    static constexpr float CURRENT_LSB(float mcr){
        return mcr / 32768.0f;
    }
    static constexpr float POWER_LSB(float c_lsb){
        return c_lsb * 25.0f;
    }
    static constexpr uint16_t CALIBRATION_REGISTER(float c_lsb, float sh_res){
        //NOTE: +0.5f makes static cast round up
        return static_cast<uint16_t>((0.00512f / (c_lsb * sh_res)) + 0.5f);
    }
}   //namespace CALIBRATION

//implements a single instance of an INA233 module over an I2C interface
class INA233 {
    public:
        INA233(
            float shunt_resistance,
            float max_current_rating,
            ADDRESS_PIN A0 = ADDRESS_PIN::GND,
            ADDRESS_PIN A1 = ADDRESS_PIN::GND,
            TwoWire& wire = Wire
        );

        //setup
        bool begin();

        //public interface
        int16_t busVoltage_raw() const;
        int16_t shuntVoltage_raw() const;
        int16_t current_raw() const;
        int16_t power_raw() const;

        float busVoltage_V(int16_t raw) const;
        float shuntVoltage_V(int16_t raw) const;
        float current_A(int16_t raw) const;
        float power_W(int16_t raw) const;
        
    private:
        //i2c interface
        TwoWire& _wire;
        const uint8_t _addr;

        //calibration constants
        float _shunt_resistance;
        float _max_current_rating;

        const float _busVoltage_lsb = CALIBRATION::BUSVOLTAGE_LSB;
        const float _shuntVoltage_lsb = CALIBRATION::SHUNTVOLTAGE_LSB;
        float _current_lsb;
        float _power_lsb;

        uint16_t _calibration_register;

        //private helper functionss
        bool writeRegister(uint8_t reg, uint16_t value);
        bool readRegister(uint8_t reg, uint16_t& value);
};

#endif      //for INA233_I2C_H