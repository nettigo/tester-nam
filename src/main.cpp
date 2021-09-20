//
// Created by viciu on 20.09.2021.
//
#include <Arduino.h>
#include "Wire.h"

#define I2C_PIN_SCL D4
#define I2C_PIN_SDA D3

#define RST_PIN D7
#define MAX_I2C_DEVICES 20

#include "tests.h"
#include "test_sds.h"

void testI2C() {
    byte scanResults[MAX_I2C_DEVICES];
    byte devCnt =0;
    Serial.println(F("Start I2C scan"));
    String I2Clist = F("");
    String I2CResults[MAX_I2C_DEVICES];

    for (uint8_t addr = 0x07; addr <= 0x7F; addr++) {
        // Address the device
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            if (devCnt < MAX_I2C_DEVICES) {
                scanResults[devCnt++] = addr;
            } else {
                Serial.print(F("No more space for I2C device, not tested address: "));
                Serial.println(addr, 16);
            }
            I2Clist += String(addr, 16);
            I2Clist += F(", ");
        }
    }
    Serial.println(F("I2C scan ended"));
    for (byte i=0; i<devCnt; i++) {
        I2CResults[i]=F("not tested");
        switch(scanResults[i]) {
            case 0x44:
            case 0x45:
                I2CResults[i] = F("HECA: ");
                if (testSHT(scanResults[i])) {
                    I2CResults[i].concat(F("OK"));
                } else {
                    I2CResults[i].concat(F("ERR"));
                }
                break;
            case 0x76:
            case 0x77:
                if (testBMP180(scanResults[i])) {
                    I2CResults[i] = F("BMP180/085 OK");
                } else if(testBMP280(scanResults[i])) {
                    I2CResults[i] = F("BMP280 OK");
                } else if(testBME280(scanResults[i])) {
                    I2CResults[i] = F("BME280 OK");
                }
                else {
                    I2CResults[i] = F("BMP/E ERR *****");
                }
                break;
        }
    }
    Serial.print(F("I2C scan results: "));
    Serial.println(I2Clist);
    for (byte i=0; i< devCnt; i++) {
        Serial.print(F(" * 0x"));
        Serial.print(scanResults[i],16);
        Serial.print(F(" "));
        Serial.println(I2CResults[i]);
    }
}

void doTests() {
    Serial.println(F("Test start....."));
    Serial.println(F("Disabling other Wemos."));
    digitalWrite(RST_PIN, LOW);

    testI2C();
    Serial.println(F("\nSDS test"));
    if(testSDS()) {
        Serial.println(F("\nSDS OK"));
    }else {
        Serial.println(F("\n**** SDS ERROR ****"));

    }

    Serial.println(F("Enabling other Wemos."));
    digitalWrite(RST_PIN, HIGH);
    Serial.println(F("Test end....."));

}


void setup() {
    Serial.begin(115200);
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, HIGH);

    Wire.begin(I2C_PIN_SDA, I2C_PIN_SCL);
    Wire.setClock(100000); // Force bus speed 100 Khz
    delay(1000 * 2);
    doTests();
    while (true) { yield(); };
}

void loop() {
    digitalWrite(RST_PIN, LOW);
    delay(2000);
    digitalWrite(RST_PIN, HIGH);
    delay(5000);

}