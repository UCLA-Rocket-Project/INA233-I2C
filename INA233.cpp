#include <INA233.h>

INA233::INA233(float shunt_resistance, float max_current_rating, ADDRESS_PIN A0, ADDRESS_PIN A1, TwoWire& wire) :
    _shunt_resistance(shunt_resistance),
    _max_current_rating(max_current_rating),
    _wire(wire),
{
    //get I2C address
    _addr = ADDRESS::TARGET_ADDRESS(A0, A1);

    //setup calibration constants
    _current_lsb = CALIBRATION::CURRENT_LSB(_max_current_rating);
    _power_lsb = CALIBRATION::POWER_LSB(_current_lsb);
    _calibration_register = CALIBRATION::CALIBRATION_REGISTER(_current_lsb, _shunt_resistance);
}

//-------- setup functions --------

bool INA233::begin(){
    if (_addr < ADDRESS::MIN_I2C_ADDR || _addr > ADDRESS::MAX_I2C_ADDR) return false;

    //write to start i2c bus then calibration register (see 6.5.2)
    return _wire.begin() && writeRegister(COMMAND::MFR_CALIBRATION, _calibration_register);
}

//-------- public interface functions --------

int16_t INA233::busVoltage_raw() const{
    uint16_t value;
    bool sts = readRegister(
        COMMAND::READ_VIN,
        value
    );

    return (sts)? (int16_t) value : 0;
}

int16_t INA233::shuntVoltage_raw() const{
    uint16_t value;
    bool sts = readRegister(
        COMMAND::MFR_READ_VSHUNT,
        value
    );

    return (sts)? (int16_t) value : 0;
}

int16_t INA233::current_raw() const{
    uint16_t value;
    bool sts = readRegister(
        COMMAND::READ_IIN,
        value
    );

    return (sts)? (int16_t) value : 0;
}

int16_t INA233::power_raw() const{
    uint16_t value;
    bool sts = readRegister(
        COMMAND::READ_PIN,
        value
    );

    return (sts)? (int16_t) value : 0;
}

float INA233::busVoltage_V(int16_t raw) const{
    return raw * _busVoltage_lsb;
}

float INA233::shuntVoltage_V(int16_t raw) const{
    return raw * _shuntVoltage_lsb;
}

float INA233::current_A(int16_t raw) const{
    return raw * _current_lsb;
}

float INA233::power_W(int16_t raw) const{
    return raw * _power_lsb;
}

//-------- private helper functions --------

bool INA233::writeRegister(uint8_t reg, uint16_t value){
    _wire.beginTransmission(_addr);
    _wire.write(reg);

    //little endian
    _wire.write(value & 0xFF);
    _wire.write((value >> 8) & 0xFF);

    return (_wire.endTransmission() == 0);
};

bool INA233::readRegister(uint8_t reg, uint16_t& value){
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