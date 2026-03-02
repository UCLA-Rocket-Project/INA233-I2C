#include <INA233.h>

INA233::INA233(float shunt_resistance, float max_current_rating, TwoWire& wire, uint8_t addr) :
    _shunt_resistance(shunt_resistance),
    _max_current_rating(max_current_rating),
    _wire(wire),
    _addr(addr) 
{
    //setup calibration constants
    _current_lsb = CALIBRATION::CURRENT_LSB(_max_current_rating);
    _power_lsb = CALIBRATION::POWER_LSB(_current_lsb);
    _calibration_register = CALIBRATION::CALIBRATION_REGISTER(_current_lsb, _shunt_resistance);
}

//-------- setup functions --------

bool INA233::begin(){
    //write to calibration register (see 6.5.2) and start i2c bus
    return writeRegister(COMMAND::MFR_CALIBRATION, _calibration_register) && _wire.begin();
}

bool INA233::close(){
    return _wire.end();
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