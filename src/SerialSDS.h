//
// Created by viciu on 10.06.2021.
//

#ifndef NAMF_SERIALSDS_H
#define NAMF_SERIALSDS_H

#define PM_SERIAL_RX D1
#define PM_SERIAL_TX D2
#include "SoftwareSerial.h"
SoftwareSerial serialSDS;//(PM_SERIAL_RX, PM_SERIAL_TX, false);

#include <Arduino.h>

class SerialSDS {
public:
    typedef enum {
        SER_UNDEF,
        SER_HDR,
        SER_DATA,
        SER_REPLY,
        SER_IDLE
    } SerialState;
    typedef enum {
        SDS_REPORTING,
        SDS_DATA,
        SDS_NEW_DEV_ID,
        SDS_SLEEP,
        SDS_PERIOD,
        SDS_FW_VER,
        SDS_UNKNOWN
    } ResponseType;
    typedef struct {
        bool sent;
        bool received;
        unsigned long lastRequest;
        unsigned long lastReply;
        byte data[5];
    } ReplyInfo;

    SerialSDS(Stream &serial) : _serial(serial) {
        _currState = SER_UNDEF;
        checksumFailed = 0;
        packetCount = 0;
        for (byte i = 0; i < SDS_UNKNOWN; i++) {
            _replies[i].sent = false;
            _replies[i].received = false;
            _replies[i].lastRequest = 0;
            _replies[i].lastReply = 0;

        }
    }

    void process();

    unsigned checksumErrCnt() { return checksumFailed; }

    unsigned long totalPacketCnt() { return packetCount; }

    bool readingAvailable();

    bool fetchReading(int &pm10, int &pm25);

    ReplyInfo _replies[SDS_UNKNOWN];


private:
    Stream &_serial;
    SerialState _currState;
    byte _buff[10];
    unsigned checksumFailed;
    unsigned long packetCount;

    bool checksumValid();

    void logReply(ResponseType type);

    void storeReply();

    ResponseType selectResponse(byte x);
//    const char SRT_0 [];
//    const char SRT_1 [];
//    const char SRT_2 [];
//    const char SRT_3 [];
//    const char SRT_4 [];
//    const char SRT_5 [] PROGMEM = "FW version";
//    const char SRT_6 [] PROGMEM = "Unknown";
//    const char *SRT_NAMES[] PROGMEM = {SRT_0, SRT_1, SRT_2, SRT_3, SRT_4, SRT_5, SRT_6};


};

SerialSDS channelSDS(serialSDS);
unsigned long pm10Sum, pm25Sum = 0;
unsigned readingCount = 0;

void resetReadings() {
    pm10Sum = pm25Sum = readingCount = 0;
};

void storeReadings(const int pm10, const int pm25) {
    if (pm10 == -1 || pm25 == -1) return;
    pm10Sum += pm10;
    pm25Sum += pm25;
    readingCount++;
};


void SerialSDS::process() {
    while (_serial.available()) {
        byte x;
        static byte idx;
        switch (_currState) {
            case SER_UNDEF:
                x = _serial.read();
                if (x == 0xAA) {
                    _currState = SER_HDR;
                    idx = 2;
                    _buff[0] = 0xAA;
                }
                break;
            case SER_HDR:
                x = _serial.read();
                _buff[1] = x;
                if (x == 0xC0) {
                    _currState = SER_DATA;
                } else if (x == 0xC5) {
                    _currState = SER_REPLY;
                } else
                    _currState = SER_UNDEF;
                break;
            case SER_REPLY:
                if (idx < 10) {
                    _buff[idx] = _serial.read();
                    idx++;
                }
                if (idx == 10) {
                    if (!checksumValid()) {
                        _currState = SER_UNDEF;
                        break;
                    }
                    storeReply();
                    _currState = SER_UNDEF;
                }
                break;
            case SER_DATA:
                if (idx < 10)
                    _buff[idx++] = _serial.read();
                if (idx == 10) {
                    if (!checksumValid()) {
                        _currState = SER_UNDEF;
                        break;
                    }
                    _replies[SDS_DATA].received = true;
                    _replies[SDS_DATA].lastReply = millis();
                    for (byte i = 0; i < 5; i++) _replies[SDS_DATA].data[i] = _buff[2 + i];
                    _currState = SER_UNDEF;
                }
                break;
            default:
                _currState = SER_UNDEF;
        }
        yield();
    }
}

bool SerialSDS::readingAvailable() {
    return _replies[SDS_DATA].received;
};

bool SerialSDS::fetchReading(int &pm10, int &pm25) {
    if (!readingAvailable()) return false;
    pm25 = _replies[SDS_DATA].data[0] | (_replies[SDS_DATA].data[1] << 8);
    pm10 = _replies[SDS_DATA].data[2] | (_replies[SDS_DATA].data[3] << 8);
    _replies[SDS_DATA].received = false;
    return true;

}

bool SerialSDS::checksumValid(void) {
    uint8_t checksum_is = 0;
    for (unsigned i = 2; i < 8; ++i) {
        checksum_is += _buff[i];
    }
    bool chk = _buff[9] == 0xAB && checksum_is == _buff[8];
    packetCount++;
    if (!chk) {
        Serial.println(F("SDS011 reply checksum failed "));
        checksumFailed++;
//        Serial.println(checksum_is,16);
//        for (byte i=0; i<10; i++) {
//            Serial.print(_buff[i],16);
//            Serial.print(" ");
//        }
//        Serial.println();
    }
    return (chk);
}


void SerialSDS::logReply(ResponseType type) {
    ReplyInfo x = _replies[type];
//    for (byte i=0; i<5;i++) {Serial.print(x.data[i],16); Serial.print(" ");}
//    Serial.println();
//    Serial.print(F("**** SDS**** Odpowiedź z SDS: "));
    switch (type) {
        case SDS_REPORTING:
            Serial.print(F("REPORTING MODE "));
            Serial.print(x.data[0] ? F("SET: ") : F("QUERY: "));
            Serial.print(x.data[1] ? F("query ") : F("active"));
            break;
        case SDS_DATA:
//            Serial.print(F("data packet"));
            break;
        case SDS_NEW_DEV_ID:
        case SDS_SLEEP:
            Serial.print(F("SLEEP MODE "));
            Serial.print(x.data[0] ? F("SET: ") : F("QUERY: "));
            Serial.print(x.data[1] ? F("work ") : F("sleep"));
            break;
        case SDS_PERIOD:
            Serial.print(F("WORKING PERIOD "));
            Serial.print(x.data[0] ? F("SET: ") : F("QUERY: "));
            Serial.print(x.data[1] ? String(x.data[1]) : F("continous"));
            break;

        case SDS_FW_VER:
            Serial.println(F("FIRMWARE VERSION response"));
            break;

    }
    Serial.println();
}

//store reply for command
void SerialSDS::storeReply() {
    ResponseType type;
    type = selectResponse(_buff[2]);
//    debug_out(F("Response type: "), DEBUG_MED_INFO, 0);
//    debug_out(String(type), DEBUG_MED_INFO);
    if (type == SDS_UNKNOWN) { return; }
    _replies[type].received = true;
    _replies[type].lastReply = millis();
    for (byte i = 0; i < 5; i++) _replies[type].data[i] = _buff[3 + i];

//    if (cfg::debug > DEBUG_MIN_INFO) logReply(type);
}

SerialSDS::ResponseType SerialSDS::selectResponse(byte x) {
    switch (x) {
        case 7:
            return SDS_FW_VER;
        case 8:
            return SDS_PERIOD;
        case 6:
            return SDS_SLEEP;
        case 5:
            return SDS_NEW_DEV_ID;
        case 2:
            return SDS_REPORTING;
        default:
            return SDS_UNKNOWN;
    }
}




#endif //NAMF_SERIALSDS_H
