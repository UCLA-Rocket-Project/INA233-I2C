#include <Arduino.h>
#include <Wire.h>
#include <INA233.h>

INA233 ina(
    0.01f,  //example shunt resistance
    10.0f,  //example maximum current rating
    ADDRESS_PIN::GND,   //A0
    ADDRESS_PIN::GND,   //A1
    Wire
);

void setup() {
    Serial.begin(115200);

    if(!ina.begin()){
        Serial.println("Setup failed.");
        while(1);
    }

    Serial.println("Setup complete.");
}

void loop() {
    int16_t raw_vin = ina.busVoltage_raw();
    float vin = ina.busVoltage_V(raw_vin);

    Serial.print("VIN = ");
    Serial.println(vin);

    delay(1000);
}