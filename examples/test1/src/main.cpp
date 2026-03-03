#include <Arduino.h>
#include <Wire.h>
#include <INA233.h>

//Ares 2025-2026 Bodytube Analog Board v1.1
#define SDA                    (int) 42             //ESP32 GPIO pins
#define SCL                    (int) 41

#define A0                      ADDRESS_PIN::GND    //both tied to GND
#define A1                      ADDRESS_PIN::GND

#define RSHUNT                  (float) 0.027f      //from ERJ3BWFR027V 27mO shunt resistor
#define MAX_CURRENT_RATING      (float) 4.0f

INA233 ina(
    RSHUNT,
    MAX_CURRENT_RATING,
    A0,
    A1,
    Wire
);

void setup() {
    delay(3000);
    Serial.begin(115200);

    Wire.begin(SDA, SCL);
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
    Serial.print(vin);

    int16_t raw_curr = ina.current_raw();
    float curr = ina.current_A(raw_curr);

    Serial.print(",\tCURR = ");
    Serial.println(curr);

    delay(1000);
}