[![arduino-library-badge](https://www.ardu-badge.com/badge/NonBlockingDallas.svg?)](https://www.ardu-badge.com/NonBlockingDallas)

# Non blocking temperature sensor library for Arduino

This simple library for Arduino implements a machine state for reading the **Maxim Integrated DS18B20 temperature sensor** without blocking the main loop() of the sketch. It is designed for a **continuous sensor reading** every amount of time configurable by the developer. It is also possible to request a new sensor reading on the fly by calling the *requestTemperature()* function.  

While the conversion is in progress, the main loop() continues to run so that the sketch can execute other tasks. When the temperature reading is ready, a callback is invoked. At full resolution the conversion time takes up to 750 milliseconds, a huge amount of time, thus the importance of the library to avoid blocking the sketch execution.

Supports up to 15 sensors on the same ONE WIRE bus. 

# Installation

## Arduino

### Prerequisites

This library uses OneWire and DallasTemperature libraries, so you will need to have those installed.

#### Litle breaking changes for new version 1.1x :

* Function `begin()` ==> remove units parameters.
* In all callback's functions ==> Parameters are not the sames.

### Include library

The library is available from the Arduino Library Manager: load the Arduino IDE, then use the menu at the top to select Sketch -> Include Library -> Manage Libraries. Type **NonBlockingDallas** in the search box.

Click the following badge for a complete installation guide

[![arduino-library-badge](https://www.ardu-badge.com/badge/NonBlockingDallas.svg?)](https://www.ardu-badge.com/NonBlockingDallas)

## PlatformIO

[See install documentation](https://registry.platformio.org/libraries/gbertaz/NonBlockingDallas/installation)

# Usage

## Step 1

Include the required libraries:

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>
```

## Step 2

Create the instance of the classes:

```cpp
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas temperatureSensors(&dallasTemp);
```

## Step 3

Initialize the sensor and set the callbacks.
The parameters of the *begin* function are the **sensor resolution**, **unit of measure** (Celsius or Fahrenheit) and **time interval** in milliseconds.

```cpp
temperatureSensors.begin(NonBlockingDallas::resolution_12, 1500);
temperatureSensors.onIntervalElapsed(handleIntervalElapsed);
temperatureSensors.onTemperatureChange(handleTemperatureChange);
temperatureSensors.onDeviceDisconnected(handleDeviceDisconnected);
```

### Sensors resolution

The conversion time of the DS18B20 temperature sensor depends on its resolution, thus the **time interval** parameter passed to the *begin* function must be greater than or equal to the conversion time.

|     enum      | Resolution  | Conversion time |
| ------------- | ----------- | --------------- |
| resolution_9  |       9 bit |           93 ms |
| resolution_10 |      10 bit |          187 ms |
| resolution_11 |      11 bit |          375 ms |
| resolution_12 |      12 bit |          750 ms |


## Step 4

Implement the callbacks' functions and call the *update* function inside the main loop()  

```cpp
void loop()
{
	temperatureSensors.update();
}

void handleIntervalElapsed(int deviceIndex, int32_t temperatureRAW)
{
}

void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW)
{
	// If needed, call on the fly conversion
	float tC = temperatureSensors.rawToCelsius(temperatureRAW);
	float tF = temperatureSensors.rawToFahrenheit(temperatureRAW);
}

void handleDeviceDisconnected(int deviceIndex)
{
}
```

### Example

Please see the [Example](https://github.com/Gbertaz/NonBlockingDallas/blob/master/examples/TemperatureReading/TemperatureReading.ino) for a complete working sketch

# Debug

To get some debug information, simply remove the comment on following line in *NonBlockingDallas.h*:

```cpp
#define DEBUG_DS18B20
```

The output will be like the following:

```log
DS18B20: 3 sensors found on the bus
DS18B20: parasite power is OFF
DS18B20: requested new reading
DS18B20 (0): 29.37 °C
DS18B20 (1): 29.12 °C
DS18B20 (2): 29.26 °C
DS18B20: requested new reading
DS18B20 (0): 29.37 °C
DS18B20 (1): 29.12 °C
DS18B20 (2): 29.26 °C
...
```

# Callbacks

The library is callback driven:
- *onIntervalElapsed* invoked **every time** the timer interval is elapsed and the sensor reading is **valid**
- *onTemperatureChange* invoked **only when the temperature value changes** between two **valid** readings of the same sensor
- *onDeviceDisconnected* invoked when the device is disconnected

In the latest version of the library I have introduced *onDeviceDisconnected* which makes the *valid* parameter meaningless. In order to maintain retro compatibility, it will always be *true*. It will be removed in a future version.
*deviceIndex* represents the index of the sensor on the bus, values are from 0 to 14.

```cpp
void onIntervalElapsed(void(*callback)(int deviceIndex, int32_t temperatureRAW)) {
	cb_onIntervalElapsed = callback;
}

void onTemperatureChange(void(*callback)(int deviceIndex, int32_t temperatureRAW)) {
	cb_onTemperatureChange = callback;
}

void onDeviceDisconnected(void(*callback)(int deviceIndex)) {
	cb_onDeviceDisconnected = callback;
}
```

# Additional functions


## By deviceIndex

```cpp
bool indexExist(uint8_t deviceIndex);
bool getDeviceAddress(uint8_t deviceIndex, DeviceAddress deviceAddress);
String getAddressString(uint8_t deviceIndex);
int32_t getTemperatureRAW(uint8_t deviceIndex);
float getTemperatureC(uint8_t deviceIndex);
float getTemperatureF(uint8_t deviceIndex);
```

## By DeviceAddress

```cpp
int8_t getIndex(DeviceAddress deviceAddress); // can be sused to test if address exist
int32_t getTemperatureRAW(DeviceAddress deviceAddress);
float getTemperatureC(DeviceAddress deviceAddress);
float getTemperatureF(DeviceAddress deviceAddress);
```

## By String address representation

```cpp
int8_t getIndex(String addressString); // can be sused to test if address exist
int32_t getTemperatureRAW(String addressString);
float getTemperatureC(String addressString);
float getTemperatureF(String addressString);
```

## Helpers

### Apply on real sensors

```cpp
uint8_t getSensorsCount();
bool validateAddressesRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, bool exclusiveListSet = true);
bool validateAddressesRange(String addressesStrings[], uint8_t numberOfAddresses, bool exclusiveListSet = true);
void mapIndexPositionOfDeviceAddressRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, int8_t mapedPositions[]);
```

### Apply on virtual data

```cpp
bool compareTowDeviceAddresses(DeviceAddress deviceAddress1, DeviceAddress deviceAddress2);
String convertDeviceAddressToString(DeviceAddress deviceAddress);
bool convertDeviceAddressStringToDeviceAddress(String addressString, DeviceAddress deviceAddress);
float rawToCelsius(int32_t rawTemperature);
float rawToFahrenheit(int32_t rawTemperature);
uint8_t charToHex(char c);
bool towCharToHex(char MSB, char LSB, uint8_t *ptrValue);
```

## Complex example of usage

See file `examples/AdditionalFunctions/AdditionalFunctions.ino`
