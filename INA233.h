#ifndef INA233_I2C_H

#define INA233_I2C_H

#include <Arduino.h>
#include <Wire.h>

class INA233 {
    public:

        INA233(TwoWire& wire = Wire, uint8_t addr = 0x40) : _wire(wire), _addr(addr) {};

        //setup
        bool begin();
        bool close();

        bool setShuntResistance_Ohm(float shunt_resistance){
            _shunt_resistance = shunt_resistance;
        };

        //instrumentation
        float busVoltage_V();
        float shuntVoltage_V();
        float current_A();
        float power_W();
        
    private:

        TwoWire& _wire;
        uint8_t _addr;

        float _current_lsb = 0.0f;
        float _power_lsb = 0.0f;
        float _shunt_resistance = 0.0f;

        float linear11ToFloat(uint16_t raw);
        uint16_t floatToLinear11(float value);

        bool writeRegister(uint8_t reg, uint16_t value){
            _wire.beginTransmission(_addr);
            _wire.write(reg);

            //little endian
            _wire.write(value & 0xFF);
            _wire.write((value >> 8) & 0xFF);

            return (_wire.endTransmission() == 0);
        };

        bool readRegister(uint8_t reg, uint16_t& value){
            _wire.beginTransmission(_addr);
            _wire.write(reg);

            if (_wire.endTransmission(false) != 0)
                return false;

            if (_wire.requestFrom(_addr, (uint8_t)2) != 2)
                return false;

            uint8_t lsb = _wire.read();
            uint8_t msb = _wire.read();

            value = (uint16_t)lsb | ((uint16_t)msb << 8);
            return true;
        };
};

#endif      //for INA233_I2C_H