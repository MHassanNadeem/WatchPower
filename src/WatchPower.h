#pragma once

/* WatchPower library by Hassan Nadeem
*/

#include "Arduino.h"
#include "Particle.h"
#include <stdint.h>

#define CRC_HIGH_BYTE( crc )   ((crc) >> 8) & 0xFF
#define CRC_LOW_BYTE( crc )    (crc) & 0xFF

#define DBG(str)                                       \
    do{                                                \
        delay(1000);  /* To avoid rate limit */        \
        Particle.publish("DEBUG "#str, String(str));   \
    }while(0)

#define strncpyTerminated(destination, source, num)   \
    do{                                               \
        strncpy(destination, source, num);            \
        destination[num] = 0;                         \
    }while(0)

class WatchPower{
public:
    /* All enums are position sensitive */
    enum class OutputSourcePriorities{UtilityFirst, SolarFirst, SBU};
    enum class ChargePriorities{UtilityFirst, SolarFirst, SolarAndUtility, SolarOnly};
    enum class BatteryTypes{AGM, Flooded, User};
    enum class GridRange{Appliance, UPS};
    enum class BatteryRechargeVoltages{V22, V22_5, V23, V23_5, V24, V24_5, V25, V25_5};
    static constexpr char *BatteryRechargeVoltages2Str[] = {
        "22", "22.5", "23", "23.5", "24", "24.5", "25", "25.5"
    };
    enum class BatteryReDischargeVoltages{Full, V25, V25_5, V26, V26_5, V27, V27_5, V28, V28_5, V29};
    static constexpr char *BatteryReDischargeVoltages2Str[] = {
        "00.0", "25", "25.5", "26", "26.5", "27", "27.5", "28", "28.5", "29"
    };
    enum class OututFrequencies{HZ50, HZ60};
    static constexpr char *OututFrequencies2Str[] = {
        "50", "60"
    };

    struct floatEntry{
        char str[10];
        float flt;
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

    struct flagEntry{
        bool buzzer;
        bool overLoadBypass;
        bool powerSaving;
        bool lcdTimeout;
        bool overloadRestart;
        bool overTemperatureRestart;
        bool backlight;
        bool alarm;
        bool faultCodeRecord;
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
                bool overTemperature:1;
                bool fanLocked:1;
                bool batteryVoltageHigh:1;
                bool batteryLowAlarm:1;
                bool reserved_a13:1;
                bool batteryUnderShutdown:1;
                bool reserved_a15:1;
                bool overLoad:1;
                bool eepromFault:1;
                bool inverterOverCurrent:1;
                bool inverterSoftFail:1;
                bool selfTestFail:1;
                bool opdcVoltageOver:1;
                bool batteryOpen:1;
                bool currentSensorFail:1;
                bool batteryShort:1;
                bool powerLimit:1;
                bool pvVoltageHigh:1;
                bool mpptOverloadFault:1;
                bool mpptOverloadWarning:1;
                bool batteryTooLowToCharge:1;
                bool reserved_a30:1;
                bool reserved_a31:1;
            } bits;
        } warning;
    };

    struct ratingEntry{
        floatEntry batteryRechargeVoltage;
        BatteryTypes batteryType;
        floatEntry maxACChargingCurrent;
        floatEntry maxChargingCurrent;
        OutputSourcePriorities outputSourcePriority;
        ChargePriorities chargePriority;
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
    /* Device flag status inquiry */
    static constexpr char *CMD_FLAG_INQUIRY = "QFLAG\x98\x74";


    HardwareSerial* refSer;
    bool conditioningEnabled = false;

    uint16_t calculateCRC(const char *ptr, int count);
    void appendCRC(char *str);
    bool validateCRC(char *str, uint16_t len);
    void clearSerialBuffer();
    uint16_t readLine(char *buffer, uint16_t length);
    void sendLine(const char *str);
    bool querySolar(const char *query, char *buffer, uint16_t bufferLen);
    bool isACK(const char *str);
    bool isNACK(const char *str);
    /* --- Parsing Functions --- */
    void parseQPIGS(const char *buffer);
    void parseQMOD(const char *buffer);
    void parseWarnings(const char *buffer);
    void parseSerialNumber(const char *buffer);
    void parseFirmwareVerPrimary(const char *buffer);
    void parseFirmwareVerSecondary(const char *buffer);
    void parseRating(const char *buffer);
    void parseFlags(const char *buffer);
    /*--------------------------------*/
    void conditionData();


public:
    char serialNumer[15];
    char firmwareVerPrimary[9];
    char firmwareVerSecondary[9];

    flagEntry flags;

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

    /* Constructor and Destructor */
    WatchPower(HardwareSerial &_refSer);
    WatchPower(HardwareSerial &_refSer, bool _conditioningEnabled);
    ~WatchPower();

    /* Get parameters */
    bool refreshData();
    bool refreshDeviceConstants();
    bool refreshSettings();

    /*  */
    bool isOnBattery();
    bool isOnGrid();
    bool isCharging();
    bool isSolarCharging();
    bool isGridCharging();
    bool isGridAvailable();
    bool isSolarAvailable();

    /* Set parameters */
    bool setOutputSourcePriority(OutputSourcePriorities prio);
    bool setBatteryType(BatteryTypes batteryType);
    bool setChargePriority(ChargePriorities prio);
    bool setBatteryRechargeVoltage(BatteryRechargeVoltages voltage);
    bool setBatteryReDischargeVoltage(BatteryReDischargeVoltages voltage);

};
