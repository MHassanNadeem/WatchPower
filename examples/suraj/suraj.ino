#include <WatchPower.h>
#include <SparkFunPhant.h>
#include <stdint.h>

#define DBG(str) Particle.publish("DEBUG "#str, String(str))

#define TIMEZONE_PAKISTAN 5

#define SECONDS 1000LU /*ms*/
#define MINUTES 60*SECONDS
#define HOURS   60*MINUTES

/* Local Time: yyyy-mm-dd hh:mm:ss */
const char *TIME_FORMAT_CUSTOM = "%EY-%Em-%Ed %X";

class WatchPower watchPower(Serial1, true);

class Runner{
public:
    unsigned long lastMillis;
    unsigned long period;
    void (*cb)(void);

    Runner(void (*cb)(void), unsigned long period){
        this->lastMillis = millis();
        this->period = period;
        this->cb = cb;
    }

    void run(){
        unsigned long currentMillis = millis();

        if(currentMillis - lastMillis > period){
            lastMillis = currentMillis;
            (*cb)();
        }
    }
};

void phantPublisher(void){
    time_t time = Time.now();
    Particle.publish("Time", Time.format(time, TIME_FORMAT_CUSTOM));

    watchPower.refreshData();

    Phant phant(PHANT_SERVER, PHANT_PUBLIC_KEY, PHANT_PRIVATE_KEY); // Create a Phant object

    phant.add("time", Time.format(time, TIME_FORMAT_CUSTOM));

    phant.add("mode", watchPower.mode);
    phant.add("gridvoltage", watchPower.gridVoltage.str);
    phant.add("gridfreq", watchPower.gridFreq.str);
    phant.add("outputvoltage", watchPower.outputVoltage.str);
    phant.add("outputfreq", watchPower.outputFreq.str);
    phant.add("outputpowerapparent", watchPower.outputPowerApparent.str);
    phant.add("outputpoweractive", watchPower.outputPowerActive.str);
    phant.add("loadpercent", watchPower.loadPercent.str);
    phant.add("busvoltage", watchPower.busVoltage.str);
    phant.add("batteryvoltage", watchPower.batteryVoltage.str);
    phant.add("batterycurrent", watchPower.batteryCurrent.str);
    phant.add("battery_capacity", watchPower.batteryCapacity.str);
    phant.add("temperature", watchPower.temperature.str);
    phant.add("solar_current", watchPower.solarCurrent.str);
    phant.add("solar_voltage", watchPower.solarVoltage.str);
    phant.add("battery_voltage_scc", watchPower.batteryVoltageSCC.str);
    phant.add("battery_dis_current", watchPower.batteryDischargeCurrent.str);
    phant.add("status", watchPower.status.str);
    phant.add("warning", watchPower.warning.str);

    phant.particlePost();
}

void ping(){
    int signalStrength = WiFi.RSSI();
    Particle.publish("SignalStrength", String(signalStrength));
}

void syncTime(){
    Particle.syncTime();
}

Runner phantPublisherRunner(&phantPublisher, 20*SECONDS); /* phant rate limit is 1 post every 10s */
Runner syncTimeRunner(&syncTime, 12*HOURS);
Runner pingRunner(&ping, 10*SECONDS);

void setup() {
    Time.zone( TIMEZONE_PAKISTAN );

    /* Delay so I have time to reflash in case of bad code in loop() function */
    delay(5*SECONDS);
}

void loop() {
    pingRunner.run();
    phantPublisherRunner.run();
    syncTimeRunner.run();
}
