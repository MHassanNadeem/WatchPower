/* WatchPower library by Hassan Nadeem
*/

#include "WatchPower.h"

WatchPower::WatchPower(HardwareSerial &_refSer){
    refSer = &_refSer;

    /* Communication format for OptiSolar
    * Baud Rate: 2400
    * Start Bit: 1
    * Data Bit: 8
    * Parity Bit: No
    * Stop Bit: 1 */
    refSer->begin( 2400 );

    delay(100); /* Wait for serial port init */

    refreshDeviceConstants();
    refreshSettings();
}

WatchPower::~WatchPower(){
    refSer->end();
}

/* CRC-CCITT (XModem)
* Source: http://web.mit.edu/6.115/www/amulet/xmodem.htm */
uint16_t WatchPower::calculateCRC(char *ptr, int count){
    int  crc;
    char i;
    crc = 0;
    while (--count >= 0){
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do{
            if (crc & 0x8000){
                crc = crc << 1 ^ 0x1021;
            }else{
                crc = crc << 1;
            }
        }while(--i);
    }
    return (crc);
}

void WatchPower::appendCRC(char *str){
    uint16_t len = strlen(str);

    uint16_t crc = calculateCRC(str, len);

    str[len++] = CRC_HIGH_BYTE( crc );
    str[len++] = CRC_LOW_BYTE( crc );
    str[len++] = 0;
}


bool WatchPower::validateCRC(char *str, uint16_t len){
    if(len < 3) return false;

    uint16_t crc = calculateCRC(str, len-2);

    return CRC_HIGH_BYTE(crc) == str[len-2] && CRC_LOW_BYTE(crc) == str[len-1];
}

bool WatchPower::isACK(const char *str){
    /* If found, skip the start byte */
    if(str[0] == '(') str++;

    return strstr(str, "ACK") == str;
}

bool WatchPower::isNACK(const char *str){
    return !isACK(str);
}


void WatchPower::clearInputBuffer(){
    while( refSer->available() ) refSer->read();
}

uint16_t WatchPower::readLine(char *buffer, uint16_t length){
    if(length < 1) return 0;

    uint16_t bytesRead = refSer->readBytesUntil('\r', buffer, length - 1);
    buffer[bytesRead] = 0;

    return bytesRead;
}

void WatchPower::sendLine(const char *str){
    refSer->print(str);
    refSer->print('\r');
}

bool WatchPower::querySolar(const char *query, char *buffer, uint16_t bufferLen){
    clearInputBuffer();
    sendLine(query);
    uint16_t bytesRead = readLine(buffer, bufferLen);
    return validateCRC(buffer, bytesRead);
}



void WatchPower::parseQPIGS(const char *buffer){
    buffer++; /* Skip start byte '(' */

    #define copyAndAdvance(dest, src, size) strncpyTerminated(dest, src, size); buffer += size+1
    copyAndAdvance(gridVoltage.str,             buffer, 5);
    copyAndAdvance(gridFreq.str,                buffer, 4);
    copyAndAdvance(outputVoltage.str,           buffer, 5);
    copyAndAdvance(outputFreq.str,              buffer, 4);
    copyAndAdvance(outputPowerApparent.str,     buffer, 4);
    copyAndAdvance(outputPowerActive.str,       buffer, 4);
    copyAndAdvance(loadPercent.str,             buffer, 3);
    copyAndAdvance(busVoltage.str,              buffer, 3);
    copyAndAdvance(batteryVoltage.str,          buffer, 5);
    copyAndAdvance(batteryCurrent.str,          buffer, 2);
    copyAndAdvance(batteryCapacity.str,         buffer, 3);
    copyAndAdvance(temperature.str,             buffer, 4);
    copyAndAdvance(solarCurrent.str,            buffer, 4);
    copyAndAdvance(solarVoltage.str,            buffer, 5);
    copyAndAdvance(batteryVoltageSCC.str,       buffer, 5);
    copyAndAdvance(batteryDischargeCurrent.str, buffer, 5);
    copyAndAdvance(status.str,                  buffer, 8);
    #undef copyAndAdvance

    /* Parse floats */
    gridVoltage.flt             = atof(gridVoltage.str);
    gridFreq.flt                = atof(gridFreq.str);
    outputVoltage.flt           = atof(outputVoltage.str);
    outputFreq.flt              = atof(outputFreq.str);
    outputPowerApparent.flt     = atof(outputPowerApparent.str);
    outputPowerActive.flt       = atof(outputPowerActive.str);
    loadPercent.flt             = atof(loadPercent.str);
    busVoltage.flt              = atof(busVoltage.str);
    batteryVoltage.flt          = atof(batteryVoltage.str);
    batteryCurrent.flt          = atof(batteryCurrent.str);
    batteryCapacity.flt         = atof(batteryCapacity.str);
    temperature.flt             = atof(temperature.str);
    solarCurrent.flt            = atof(solarCurrent.str);
    solarVoltage.flt            = atof(solarVoltage.str);
    batteryVoltageSCC.flt       = atof(batteryVoltageSCC.str);
    batteryDischargeCurrent.flt = atof(batteryDischargeCurrent.str);

    /* Parse status */
    status.status.byte = 0;
    for(int i=0; i<8; i++){
        status.status.byte |= (status.str[7-i] - '0')<<i;
    }
}

void WatchPower::parseQMOD(const char *buffer){
    buffer++; /* Skip start byte '(' */

    mode = buffer[0];
}

void WatchPower::parseWarnings(const char *buffer){
    buffer++; /* Skip start byte '(' */

    strncpyTerminated(warning.str, buffer, 32);

    /* Parse warnings */
    warning.warning.word = 0;
    for(int i=0; i<32; i++){
        warning.warning.word |= (warning.str[31-i] - '0')<<i;
    }
}

bool WatchPower::refreshData(){
    bool error = false;
    char inputBuffer[256];

    error |= querySolar(CMD_GENERAL_STATUS, inputBuffer, sizeof(inputBuffer));
    parseQPIGS(inputBuffer);
    // Particle.publish("_QPIGS", inputBuffer);

    error |= querySolar(CMD_MODE_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseQMOD(inputBuffer);
    // Particle.publish("_QMOD", inputBuffer);

    error |= querySolar(CMD_WARNING_STATUS, inputBuffer, sizeof(inputBuffer));
    parseWarnings(inputBuffer);
    // Particle.publish("Warn", inputBuffer);

    return error;
}

bool WatchPower::refreshDeviceConstants(){
    bool error = false;
    char inputBuffer[256];

    error |= querySolar(CMD_SERIAL_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseSerialNumber(inputBuffer);

    error |= querySolar(CMD_FIRMWARE_PRIM_VER_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseFirmwareVerPrimary(inputBuffer);

    error |= querySolar(CMD_FIRMWARE_SEC_VER_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseFirmwareVerSecondary(inputBuffer);

    return error;
}

bool WatchPower::refreshSettings(){
    bool error = false;
    char inputBuffer[256];

    error |= querySolar(CMD_FLAG_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseFlags(inputBuffer);

    error |= querySolar(CMD_RATING_INQUIRY, inputBuffer, sizeof(inputBuffer));
    parseRating(inputBuffer);

    return error;
}

bool WatchPower::isCharging(){
    return status.status.bits.chargingStatus;
}

bool WatchPower::isSolarCharging(){
    return status.status.bits.sccChargingStatus;
}

bool WatchPower::isGridCharging(){
    return status.status.bits.acChargingStatus;
}

bool WatchPower::isOnBattery(){
    return mode == 'B';
}

bool WatchPower::isOnGrid(){
    return !isOnBattery();
}


bool WatchPower::setOutputSourcePriority(OutputSourcePriorities prio){
    char command[50];
    char inputBuffer[25];

    /* Make Command */
    sprintf(command, "POP%02u", prio);
    appendCRC(command);

    int error = querySolar(command, inputBuffer, sizeof(inputBuffer));

    return (error == false && isACK(inputBuffer));
}

bool WatchPower::setChargePriority(ChargePriorities prio){
    char command[50];
    char inputBuffer[25];

    /* Make Command */
    sprintf(command, "PCP%02u", prio);
    appendCRC(command);

    int error = querySolar(command, inputBuffer, sizeof(inputBuffer));

    return (error == false && isACK(inputBuffer));
}

bool WatchPower::setBatteryRechargeVoltage(BatteryRechargeVoltages voltage){
    char command[50];
    char inputBuffer[25];

    /* Make Command */
    sprintf(command, "PBCV%s", BatteryRechargeVoltages2Str[(int)voltage]);
    appendCRC(command);

    int error = querySolar(command, inputBuffer, sizeof(inputBuffer));

    return (error == false && isACK(inputBuffer));
}

bool WatchPower::setBatteryReDischargeVoltage(BatteryReDischargeVoltages voltage){
    char command[50];
    char inputBuffer[25];

    /* Make Command */
    sprintf(command, "PBCV%s", BatteryReDischargeVoltages2Str[(int)voltage]);
    appendCRC(command);

    int error = querySolar(command, inputBuffer, sizeof(inputBuffer));

    return (error == false && isACK(inputBuffer));
}

bool WatchPower::setBatteryType(BatteryTypes batteryType){
    char command[50];
    char inputBuffer[25];

    /* Make Command */
    sprintf(command, "PBT%02u", batteryType);
    appendCRC(command);

    int error = querySolar(command, inputBuffer, sizeof(inputBuffer));

    return (error == false && isACK(inputBuffer));
}


void WatchPower::parseSerialNumber(const char *buffer){
    strncpyTerminated(serialNumer, buffer+1,14);
}

void WatchPower::parseFirmwareVerPrimary(const char *buffer){
    strncpyTerminated(firmwareVerPrimary, buffer+7,8);
}

void WatchPower::parseFirmwareVerSecondary(const char *buffer){
    strncpyTerminated(firmwareVerSecondary, buffer+8,8);
}


void WatchPower::parseFlags(const char *buffer){
    bool isEnabled = true;
    buffer++; /* Skip start byte '(' */

    #define CASE_MAKER(val,field) \
    case val:                     \
        field = isEnabled;        \
        break                     \

    for(int i=0; i<11; i++){
        switch(buffer[i]){
        case 'E':
            isEnabled = true;
            break;
        case 'D':
            isEnabled = false;
            break;
            CASE_MAKER('a', flags.buzzer);
            CASE_MAKER('b', flags.overLoadBypass);
            CASE_MAKER('j', flags.powerSaving);
            CASE_MAKER('k', flags.lcdTimeout);
            CASE_MAKER('u', flags.overloadRestart);
            CASE_MAKER('v', flags.overTemperatureRestart);
            CASE_MAKER('x', flags.backlight);
            CASE_MAKER('y', flags.alarm);
            CASE_MAKER('z', flags.faultCodeRecord);
        }
    }

    #undef CASE_MAKER
}

void WatchPower::parseRating(const char *buffer){
    DBG(buffer);
}
