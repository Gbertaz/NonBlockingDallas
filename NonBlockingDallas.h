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
#define ONE_WIRE_MAX_DEV 15			//Maximum number of devices on the One wire bus
//#define DEBUG_DS18B20

class NonBlockingDallas {

public:

	enum resolution {
		resolution_9 = 9,
		resolution_10 = 10,
		resolution_11 = 11,
		resolution_12 = 12
	};

	enum unitsOfMeasure {
		unit_C,
		unit_F
	};

	NonBlockingDallas(DallasTemperature *dallasTemp);
	void begin(resolution res, unitsOfMeasure uom, unsigned long tempInterval);
	void update();
	void requestTemperature();
	void onIntervalElapsed(void(*callback)(float temperature, bool valid, int deviceIndex)) {
		cb_onIntervalElapsed = callback;
	}
	void onTemperatureChange(void(*callback)(float temperature, bool valid, int deviceIndex)) {
		cb_onTemperatureChange = callback;
	}

private:

	enum sensorState {
		notFound = 0,
		waitingNextReading,
		waitingConversion,
		readingSensor
	};

	DallasTemperature *_dallasTemp;
	sensorState _currentState;
	uint8_t _sensorsCount;					//Number of sensors found on the bus
	unsigned long _lastReadingMillis;		//Time at last temperature sensor readout
	unsigned long _startConversionMillis;	//Time at start conversion of the sensor
	unsigned long _conversionMillis;		//Sensor conversion time based on the resolution [milliseconds]

	unsigned long _tempInterval;			//Interval among each sensor reading [milliseconds]
	unitsOfMeasure _unitsOM;				//Unit of measurement
	float _temperatures[ONE_WIRE_MAX_DEV];	//Array of last temperature values
	DeviceAddress _sensorAddresses[ONE_WIRE_MAX_DEV];

	void waitNextReading();
	void waitConversion();
	void readSensors();
	void readTemperatures(int deviceIndex);
	void(*cb_onIntervalElapsed)(float temperature, bool valid, int deviceIndex);
	void(*cb_onTemperatureChange)(float temperature, bool valid, int deviceIndex);
};

#endif