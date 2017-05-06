# WatchPower

A Particle library for interfacing with Solar Inverters that work with WatchPower PC software.
Tested on Particle Photon. Should work any arduino like (wiring) device with little to no modifications.

## Usage

Connect Serial1 of your Particle Photon / Arduino to the solar Inverter.

```
#include "WatchPower.h"
WatchPower watchPower(Serial1);

void setup() {
  Serial.begin(9600);
}

void loop() {
  watchPower.refreshData();
  Serial.println(watchPower.gridVoltage.str);
}
```

See the [examples](examples) folder for more details.

## Documentation

TODO: Describe `WatchPower`

## Contributing

Here's how you can make changes to this library and eventually contribute those changes back.

To get started, [clone the library from GitHub to your local machine](https://help.github.com/articles/cloning-a-repository/).

Modify the sources in <src> and <examples> with the new behavior.

Compile and test.

Create a [GitHub pull request](https://help.github.com/articles/about-pull-requests/) with your changes.

## LICENSE
Copyright 2017 Hassan Nadeem

Licensed under the GLP license
