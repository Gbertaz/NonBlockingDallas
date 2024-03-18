// MIT License
//
// Copyright(c) 2021 Giovanni Bertazzoni <nottheworstdev@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef NonBlockingDallas_h
#define NonBlockingDallas_h

#include <Arduino.h>
#include <DallasTemperature.h>
#define DEFAULT_INTERVAL 30000
#define ONE_WIRE_MAX_DEV 15 // Maximum number of devices on the One wire bus
// #define DEBUG_DS18B20

class NonBlockingDallas
{

public:
	enum resolution
	{
		resolution_9 = 9,
		resolution_10 = 10,
		resolution_11 = 11,
		resolution_12 = 12
	};

	NonBlockingDallas(DallasTemperature *dallasTemp);
	void begin(resolution res, unsigned long tempInterval);
	void update();
	void requestTemperature();
	void onIntervalElapsed(void (*callback)(int deviceIndex, int32_t temperatureRAW))
	{
		cb_onIntervalElapsed = callback;
	}
	void onTemperatureChange(void (*callback)(int deviceIndex, int32_t temperatureRAW))
	{
		cb_onTemperatureChange = callback;
	}
	void onDeviceDisconnected(void (*callback)(int deviceIndex))
	{
		cb_onDeviceDisconnected = callback;
	}

	uint8_t getSensorsCount();

	/**
	 * Functions below get by deviceIndex
	 */

	bool indexExist(uint8_t deviceIndex);
	bool getDeviceAddress(uint8_t deviceIndex, DeviceAddress deviceAddress);
	String getAddressString(uint8_t deviceIndex);
	int32_t getTemperatureRAW(uint8_t deviceIndex);
	float getTemperatureC(uint8_t deviceIndex);
	float getTemperatureF(uint8_t deviceIndex);

	/**
	 * Functions below get by DeviceAddress
	 */

	int8_t getIndex(DeviceAddress deviceAddress); // can be sused to test if address exist
	int32_t getTemperatureRAW(DeviceAddress deviceAddress);
	float getTemperatureC(DeviceAddress deviceAddress);
	float getTemperatureF(DeviceAddress deviceAddress);

	/**
	 * Functions below get by String address representation
	 */

	int8_t getIndex(String addressString); // can be sused to test if address exist
	int32_t getTemperatureRAW(String addressString);
	float getTemperatureC(String addressString);
	float getTemperatureF(String addressString);

	/**
	 * Functions below are helpers
	 */

	bool compareTowDeviceAddresses(DeviceAddress deviceAddress1, DeviceAddress deviceAddress2);
	String convertDeviceAddressToString(DeviceAddress deviceAddress);
	bool convertDeviceAddressStringToDeviceAddress(String addressString, DeviceAddress deviceAddress);
	float rawToCelsius(int32_t rawTemperature);
	float rawToFahrenheit(int32_t rawTemperature);
	bool validateAddressesRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, bool exclusiveListSet = true);
	bool validateAddressesRange(String addressesStrings[], uint8_t numberOfAddresses, bool exclusiveListSet = true);
	uint8_t charToHex(char c);
	bool towCharToHex(char MSB, char LSB, uint8_t *ptrValue);
	void mapIndexPositionOfDeviceAddressRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, int8_t mapedPositions[]);

private:
	enum sensorState
	{
		notFound = 0,
		waitingNextReading,
		waitingConversion,
		readingSensor
	};

	DallasTemperature *_dallasTemp;
	sensorState _currentState;
	uint8_t _sensorsCount;				  // Number of sensors found on the bus
	unsigned long _lastReadingMillis;	  // Time at last temperature sensor readout
	unsigned long _startConversionMillis; // Time at start conversion of the sensor
	unsigned long _conversionMillis;	  // Sensor conversion time based on the resolution [milliseconds]

	unsigned long _tempInterval;			 // Interval among each sensor reading [milliseconds]
	int32_t _temperatures[ONE_WIRE_MAX_DEV]; // Array of last valid temperature raw values
	DeviceAddress _sensorAddresses[ONE_WIRE_MAX_DEV];

	void waitNextReading();
	void waitConversion();
	void readSensors();
	void readTemperatures(int deviceIndex);
	void (*cb_onDeviceDisconnected)(int deviceIndex);
	void (*cb_onIntervalElapsed)(int deviceIndex, int32_t temperatureRAW);	 // Invoked only if reading is valid. "valid" parameter will be removed in a future version
	void (*cb_onTemperatureChange)(int deviceIndex, int32_t temperatureRAW); // Invoked only if reading is valid. "valid" parameter will be removed in a future version
};

#endif