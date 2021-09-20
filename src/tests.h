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

#define PM_SERIAL_RX D1
#define PM_SERIAL_TX D2

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

SoftwareSerial serialSDS;//(PM_SERIAL_RX, PM_SERIAL_TX, false);

Sds011Async<SoftwareSerial> sds011(serialSDS);
constexpr int pm_tablesize = 5;
int pm25_table[pm_tablesize];
int pm10_table[pm_tablesize];
bool gotData = false;

bool testSDS() {


    serialSDS.begin(9600, SWSERIAL_8N1, PM_SERIAL_RX, PM_SERIAL_TX, false);

    if (!sds011.set_sleep(false)) return false;

    uint16_t device_id;
    String firmware_version;
    if (!sds011.device_info(firmware_version, device_id)) {
        return false;
    }

    Sds011::Report_mode report_mode;
    if (!sds011.get_data_reporting_mode(report_mode)) {
        Serial.println("Sds011::get_data_reporting_mode() failed");
        return false;
    }
    if (Sds011::REPORT_ACTIVE != report_mode) {
        Serial.println("Turning on Sds011::REPORT_ACTIVE reporting mode");
        if (!sds011.set_data_reporting_mode(Sds011::REPORT_ACTIVE)) {
            Serial.println("Sds011::set_data_reporting_mode(Sds011::REPORT_ACTIVE) failed");
            return false;
        }
    }


    sds011.on_query_data_auto_completed([](int n) {
        Serial.println("Begin Handling SDS011 query data");
        int pm25;
        int pm10;
        Serial.print("n = ");
        Serial.println(n);
        if (sds011.filter_data(n, pm25_table, pm10_table, pm25, pm10) &&
            !isnan(pm10) && !isnan(pm25)) {
            Serial.print("PM10: ");
            Serial.println(float(pm10) / 10);
            Serial.print("PM2.5: ");
            Serial.println(float(pm25) / 10);
            gotData = true;
        }
        Serial.println("End Handling SDS011 query data");
    });

    if (!sds011.query_data_auto_async(pm_tablesize, pm25_table, pm10_table)) {
        Serial.println("measurement capture start failed");
        return false;
    } else {
        Serial.println(F("SDS: waiting for data"));
    }
    unsigned long waitTime = millis();
    while (millis() - waitTime < pm_tablesize * 1000 + 5000) {
        delay(200);
        Serial.print(F("."));
        serialSDS.perform_work();
    }

//    sds011.set_sleep(true);
    return gotData;

}

#endif //TESTER_NAM_TESTS_H
