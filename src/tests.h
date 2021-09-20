//
// Created by viciu on 20.09.2021.
//

#ifndef TESTER_NAM_TESTS_H
#define TESTER_NAM_TESTS_H

#include "ClosedCube_SHT31D.h" // support for Nettigo Air Monitor HECA
#include <Adafruit_BMP085.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include "SoftwareSerial.h"
#include <Sds011.h>

bool testSHT(byte addr) {
    ClosedCube_SHT31D heca;
    heca.begin(addr);
    if (heca.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_1HZ) != SHT3XD_NO_ERROR) {
        Serial.println(F(" [SHT ERROR] Cannot start periodic mode"));
        return false;
    }
    return true;
}

bool testBMP180(byte addr) {
    Adafruit_BMP085 bmp180;
    if (bmp180.begin(addr)) {
        return true;
    } else
        return false;

}

bool testBMP280(byte addr) {
    Adafruit_BMP280 bmp280;
    if (bmp280.begin(addr)) {
        return true;
    } else
        return false;

}


bool testBME280(byte addr) {
    Adafruit_BME280 bme280;

    if (bme280.begin(addr)) {
        return true;
    } else
        return false;

}


#endif //TESTER_NAM_TESTS_H
