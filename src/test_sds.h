//
// Created by viciu on 20.09.2021.
//



#ifndef TESTER_NAM_TEST_SDS_H
#define TESTER_NAM_TEST_SDS_H

#include "SerialSDS.h"

#define SDS_SERIAL_BUFF_SIZE 32
typedef enum  {
    Start,
    Stop,
    ContinuousMode,
    VersionDate,
    None
}PmSensorCmd;

void clearIncoming() {
    serialSDS.flush();
    if (byte avail = serialSDS.available()) {
        while(avail--) {serialSDS.read();}
    }
}
bool SDS_cmd(PmSensorCmd cmd) {
    static constexpr uint8_t start_cmd[] PROGMEM = {
            0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0x06, 0xAB
    };
    static constexpr uint8_t stop_cmd[] PROGMEM = {
            0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0x05, 0xAB
    };
    static constexpr uint8_t continuous_mode_cmd[] PROGMEM = {
            0xAA, 0xB4, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0x07, 0xAB
    };
    static constexpr uint8_t continuous_mode_cmd2[] PROGMEM = {
            0xAA, 0xB4, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0x01, 0xAB
    };
    static constexpr uint8_t version_cmd[] PROGMEM = {
            0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0x05, 0xAB
    };
    constexpr uint8_t cmd_len = 19;

    uint8_t buf[cmd_len];
    switch (cmd) {
        case PmSensorCmd::Start:
//                Serial.println(F("SDS cmd: start"));
            memcpy_P(buf, start_cmd, cmd_len);
            break;
        case PmSensorCmd::Stop:
//                Serial.println(F("SDS cmd: stop"));
            memcpy_P(buf, stop_cmd, cmd_len);
            break;
        case PmSensorCmd::ContinuousMode:
//                Serial.println(F("SDS cmd: continuous"));
            memcpy_P(buf, continuous_mode_cmd, cmd_len);
            break;
        case PmSensorCmd::VersionDate:
//                Serial.println(F("SDS cmd: version"));
            memcpy_P(buf, version_cmd, cmd_len);
            break;
    }
    clearIncoming();
    serialSDS.write(buf, cmd_len);
    return cmd != PmSensorCmd::Stop;
}



bool SDS_checksum_valid(const uint8_t (&data)[8]) {
    uint8_t checksum_is = 0;
    for (unsigned i = 0; i < 6; ++i) {
        checksum_is += data[i];
    }
    return (data[7] == 0xAB && checksum_is == data[6]);
}


void byteReceived(int cnt) {
//    Serial.print(F("SDS - received: "));
    Serial.println(cnt);
    if (cnt>=SDS_SERIAL_BUFF_SIZE) {
        Serial.println(F("SDS buffer full!"));
    }
    channelSDS.process();
}

int pm10, pm25 = 0;

bool testSDS() {


    serialSDS.begin(9600, SWSERIAL_8N1, PM_SERIAL_RX, PM_SERIAL_TX, false);
    serialSDS.onReceive(byteReceived);
    resetReadings();

    delay(100);
    serialSDS.flush();  //d
    SDS_cmd(PmSensorCmd::Start);
    delay(200);
    unsigned long start = millis();

    while(millis() - start < 5000) {
        serialSDS.perform_work();
        if (channelSDS.readingAvailable()) {
            channelSDS.fetchReading(pm10, pm25);
            storeReadings(pm10, pm25);
        }
        delay(1);
    }
    SDS_cmd(PmSensorCmd::Start);

    if (readingCount>0) {
        Serial.print(F("PM10: "));
        Serial.print(pm10Sum / readingCount/10.0);
        Serial.print(F(" PM25: "));
        Serial.print(pm25Sum / readingCount/10.0);
        return true;
    }
    return false;
}


#endif //TESTER_NAM_TEST_SDS_H
