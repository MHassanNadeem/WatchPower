// Example usage for WatchPower library by Hassan Nadeem.

#include "WatchPower.h"

// Attach Serial1 to WatchPower
WatchPower watchPower(Serial1);

void setup(){
    Serial.begin(9600);

    /* One time initialization */
    watchPower.setOutputSourcePriority(WatchPower::OutputSourcePriorities::SolarFirst);
    watchPower.setChargePriority(WatchPower::ChargePriorities::SolarFirst);
}

void loop(){
    /* Refresh the stats */
    watchPower.refreshData();

    /* Print stats from the inverter to the serial console */
    Serial.print("Solar Current: ");
    Serial.println(watchPower.solarCurrent.str);

    Serial.print("Solar Voltage: ");
    Serial.println(watchPower.solarVoltage.str);

    delay(2*1000);
}
