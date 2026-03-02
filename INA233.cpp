#include <INA233.h>

#include <cmath>

//-------- setup functions --------

bool INA233::begin(){
    _wire.begin();

    _wire.beginTransmission(_addr);
    return (_wire.endTransmission() == 0);  //see if device ACKs
}

bool INA233::close(){
    return _wire.end();
}

//-------- instrumentation functions --------

float INA233::busVoltage_V(){
    return 0.0f;
}

float INA233::shuntVoltage_V(){
    return 0.0f;
}

float INA233::current_A(){
    return 0.0f;
}

float INA233::power_W(){
    return 0.0f;
}

//-------- private functions --------

/**
 * PMBus Linear11 format:
 * 
 * [15:11] exponent (signed 5-bit)
 * [10:0]  mantissa (signed 11-bit)
 */

float INA233::linear11ToFloat(uint16_t raw){
    int16_t exponent = (raw >> 11) & 0x1F;
    if (exponent & 0x10) exponent |= 0xFFE0; //sign extend 5-bit

    int16_t mantissa = raw & 0x07FF;
    if (mantissa & 0x0400) mantissa |= 0xF800; //sign extend 11-bit

    return mantissa * powf(2.0f, exponent);
}

uint16_t INA233::floatToLinear11(float value){
    int8_t exponent = 0;
    float mantissa = value;

    while (fabs(mantissa) > 1023.0f){
        mantissa /= 2.0f;
        exponent++;
    }

    int16_t m = (int16_t) mantissa;
    return ((exponent & 0x1F) << 11) | (m & 0x07FF);
}