#pragma once

/* WatchPower library by Hassan Nadeem
 */

#include "Particle.h"
#include <stdint.h>
#include "application.h"
#include "Stream.h"

#define CRC_HIGH_BYTE( crc )   ((crc) >> 8) & 0xFF
#define CRC_LOW_BYTE( crc )    (crc) & 0xFF

#define DBG(str) Particle.publish("DEBUG "#str, String(str))

#define strncpyTerminated(destination, source, num)   \
    do{                                               \
        strncpy(destination, source, num);            \
        destination[num] = 0;                         \
    }while(0)

class WatchPower{
    struct floatEntry{
        char str[10];
        float flt;
    };

    // hxn todo
    struct modeEntry{
        char c;
        bool battery;
    };

    struct statusEntry{
        char str[8+1]; /* 8 bits */
        union{
            uint8_t byte;
            struct{
                bool acChargingStatus:1;
                bool sccChargingStatus:1;
                bool chargingStatus:1;
                bool reserved_b3:1;
                bool loadStatus:1;
                bool sscVersion:1;
                bool configStatus:1;
                bool reserved_b7:1;
            } bits;
        } status;
    };

    struct warningEntry{
        char str[32+1]; /* 32 bits */
        union{
            uint32_t word;
            struct{
                bool reserved_a0:1;
                bool inverterFault:1;
                bool busOver:1;
                bool busUnder:1;
                bool busSoftFail:1;
                bool lineFail:1;
                bool opvShort:1;
                bool inverterVoltageTooLow:1;
                bool interverVoltageTooHigh:1;
                bool overTemperature;
                bool fanLocked;
                bool batteryVoltageHigh;
                bool batteryVoltageLow;
                bool reserved_a13;
                bool overLoad;
                bool eepromFault;
                bool inverterOverCurrent;
                bool inverterSoftFail;
                bool selfTestFail;
                bool dcVoltageOver;
                bool batteryOpen;
                bool currentSensorFail;
                bool batteryShort;
                bool powerLimit;
                bool pvVoltageHigh;
                bool mpptOverload;
            } bits;
        } warning;
    };

    private:
    /* Device general status parameters inquiry */
    static constexpr char *CMD_GENERAL_STATUS = "QPIGS\xB7\xA9";
    /* Device Mode inquiry */
    static constexpr char *CMD_MODE_INQUIRY = "QMOD\x49\xC1";
    /* Device Warning Status inquiry */
    static constexpr char *CMD_WARNING_STATUS = "QPIWS\xB4\xDA";
    /* The device serial number inquiry */
    static constexpr char *CMD_SERIAL_INQUIRY = "QID\xD6\xEA";
    /* Main CPU Firmware version inquiry */
    static constexpr char *CMD_FIRMWARE_PRIM_VER_INQUIRY = "QVFW\x62\x99";
    /* Another CPU Firmware version inquiry  */
    static constexpr char *CMD_FIRMWARE_SEC_VER_INQUIRY = "QVFW2\xC3\xF5";
    /* Device Rating Information inquiry */
    static constexpr char *CMD_RATING_INQUIRY = "QPIRI\xF8\x54";


    HardwareSerial* refSer;

    uint16_t calculateCRC(char *ptr, int count);
    void appendCRC(char *str);
    bool validateCRC(char *str, uint16_t len);
    void clearInputBuffer();
    uint16_t readLine(char *buffer, uint16_t length);
    void sendLine(const char *str);
    bool querySolar(const char *query, char *buffer, uint16_t bufferLen);
    void parseQPIGS(const char *buffer);
    void parseQMOD(const char *buffer);
    void parseQPIWS(const char *buffer);
    /*--------------------------------*/
    bool isCharging();
    bool isSolarCharging();
    bool isGridCharging();

    public:
      char serialNumer[15];
      char firmwareVerPrimary[9];
      char firmwareVerSecondary[9];

      /* QMODE */
      char mode;
      /* QPIGS */
      floatEntry gridVoltage;
      floatEntry gridFreq;
      floatEntry outputVoltage;
      floatEntry outputFreq;
      floatEntry outputPowerApparent;
      floatEntry outputPowerActive;
      floatEntry loadPercent;
      floatEntry busVoltage;
      floatEntry batteryVoltage;
      floatEntry batteryCurrent;
      floatEntry batteryCapacity;
      floatEntry temperature;
      floatEntry solarCurrent;
      floatEntry solarVoltage;
      floatEntry batteryVoltageSCC;
      floatEntry batteryDischargeCurrent;
      statusEntry status;
      warningEntry warning;

      WatchPower(HardwareSerial &_refSer);
      ~WatchPower();

      bool collectData();

      void parseSerialNumber(const char *buffer){
        strncpyTerminated(serialNumer, buffer+1,14);
      }

      void parseFirmwareVerPrimary(const char *buffer){
        strncpyTerminated(firmwareVerPrimary, buffer+7,8);
      }

      void parseFirmwareVerSecondary(const char *buffer){
        strncpyTerminated(firmwareVerSecondary, buffer+8,8);
      }

      void oneTimeData(){
        char inputBuffer[256];

        querySolar(CMD_SERIAL_INQUIRY, inputBuffer, sizeof(inputBuffer));
        parseSerialNumber(inputBuffer);

        querySolar(CMD_FIRMWARE_PRIM_VER_INQUIRY, inputBuffer, sizeof(inputBuffer));
        parseFirmwareVerPrimary(inputBuffer);

        querySolar(CMD_FIRMWARE_SEC_VER_INQUIRY, inputBuffer, sizeof(inputBuffer));
        parseFirmwareVerSecondary(inputBuffer);
      }

      void parseRating(const char *buffer){
        char inputBuffer[256];

        querySolar(CMD_RATING_INQUIRY, inputBuffer, sizeof(inputBuffer));

        DBG(inputBuffer);
      }
};
