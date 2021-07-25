# Non blocking temperature sensor library

This simple library implements a machine state for reading the Maxim Integrated DS18B20 temperature sensor without blocking the main loop() of the sketch. It is designed for a continuos sensor reading every amount of time configurable by the developer by passing the time interval to the constructor of the class among with the sensor resolution and the unit of measure.
While the conversion is in progress, the main loop() continues to run so that the sketch can execute other tasks. When the temperature reading is ready, a callback is invoked. At full resolution the conversion time takes up to 750 milliseconds, a huge amount of time, thus the importance of the library to avoid blocking the sketch execution.

# Sensor Resolution

The conversion time depends upon the resolution of the sensor, thus the time interval must be greater than the conversion time.

| Resolution  | Convertion time |
| ------------- | ------------- |
| 9 bit  | 93 ms  |
| 10 bit  | 187 ms  |
| 11 bit  | 375 ms  |
| 12 bit  | 750 ms  |

# Callbacks

 The library is callback driven: *onIntervalElapsed* is invoked every time the timer interval is elapsed, *onTemperatureChange* is invoked only when the temperature changes.
 
 ```
 void onIntervalElapsed(void(*callback)(float temperature, bool valid)) {
		cb_onIntervalElapsed = callback;
	}
 
	void onTemperatureChange(void(*callback)(float temperature, bool valid)) {
		cb_onTemperatureChange = callback;
	}
 ```

# Prerequisites

This library uses OneWire and DallasTemperature libraries, so you will need to have those installed.

# Usage

### Step 1

Include the required libreries:

```
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>
```

### Step 2

Create the instance of the classes:

```
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas sensorDs18b20(dallasTemp);
```

### Step 3

Initialize the sensor and set the callbacks. 
The parameters of the begin function are the sensor resolution, unit of measure and time interval in milliseconds.

```
sensorDs18b20.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, 1500);
```

Possible values are:

* resolution_9
* resolution_10
* resolution_11
* resolution_12

* unit_C for degrees Celsius
* unit_F for Fahrenheit


### Step 4

Implement the callbacks' functions and call the update() inside the main loop()  

```

void loop() {
 sensorDs18b20.update();
}

void handleIntervalElapsed(float temperature, bool valid){

}

void handleTemperatureChange(float temperature, bool valid){

}
```

***

Please see the [Example](https://github.com/Gbertaz/NonBlockingDallas/blob/master/examples/TemperatureReading/TemperatureReading.ino) for a complete working sketch
