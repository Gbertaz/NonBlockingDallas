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

#include "NonBlockingDallas.h"

NonBlockingDallas::NonBlockingDallas(DallasTemperature *dallasTemp){
	_dallasTemp = dallasTemp;
	_sensorsCount = 0;
	_lastReadingMillis = 0;
	_startConversionMillis = 0;
	_conversionMillis = 0;
	_currentState = notFound;
	cb_onIntervalElapsed = NULL;
	cb_onTemperatureChange = NULL;
	for(int i = 0; i < ONE_WIRE_MAX_DEV; i++) _temperatures[i] = 999;
}

void NonBlockingDallas::begin(resolution res, unitsOfMeasure uom, unsigned long tempInterval){
	_tempInterval = tempInterval;
	_unitsOM = uom;
	_currentState = notFound;
	_conversionMillis = 750 / (1 << (12 - (uint8_t)res));	//Rough calculation of sensors conversion time
	_dallasTemp->begin();
	delay(50);
	_dallasTemp->setWaitForConversion(false);	//Avoid blocking the CPU waiting for the sensors conversion
	_sensorsCount = _dallasTemp->getDeviceCount();
	
	if(_sensorsCount > 0){
		_currentState = waitingNextReading;
		_dallasTemp->setResolution((uint8_t)res);
		for(int i = 0; i < _sensorsCount; i++){
			_dallasTemp->getAddress(_sensorAddresses[i], i);
		}
	}

	if ((_tempInterval < _conversionMillis) || (_tempInterval > 4294967295UL)) {
		_tempInterval = DEFAULT_INTERVAL;

		#ifdef DEBUG_DS18B20
			Serial.print("DS18B20: temperature interval not valid. Setting the default value: ");
			Serial.println(DEFAULT_INTERVAL);
		#endif
	}

	#ifdef DEBUG_DS18B20
		Serial.print("DS18B20: ");
		Serial.print(_sensorsCount);
		Serial.println(" sensors found on the bus");
		if(_sensorsCount > 0){
			Serial.print("DS18B20: parasite power is "); 
			if (_dallasTemp->isParasitePowerMode()) Serial.print("ON");
			else Serial.print("OFF");
		}
		Serial.println("");
	#endif
}

//==============================================================================================
//									PRIVATE
//==============================================================================================

void NonBlockingDallas::waitNextReading(){
	if(_lastReadingMillis != 0 && (millis() - _lastReadingMillis < _tempInterval - _conversionMillis)) return;
	requestTemperature();
}

void NonBlockingDallas::waitConversion(){
	if(_dallasTemp->isConversionComplete()){
		//Save the actual sensor conversion time to precisely calculate the next reading time
		_conversionMillis = millis() - _startConversionMillis;
		_currentState = readingSensor;
	}
}

void NonBlockingDallas::readSensors(){
	for(int i = 0; i < _sensorsCount; i++){
		readTemperatures(i);
	}

	_lastReadingMillis = millis();
	_currentState = waitingNextReading;
}

void NonBlockingDallas::readTemperatures(int deviceIndex){
	float temp = 0;
	int32_t rawTemp = _dallasTemp->getTemp(_sensorAddresses[deviceIndex]);

	if(rawTemp == DEVICE_DISCONNECTED_RAW){
		if(cb_onDeviceDisconnected)(*cb_onDeviceDisconnected)(deviceIndex);
		return;
	}

	switch(_unitsOM){
		case unit_C:
			temp = (float) rawTemp * 0.0078125f;
		break;
		case unit_F:
			temp = ((float) rawTemp * 0.0140625f) + 32.0f;
		break;
	}

	//Invoked only if reading is valid. "valid" parameter will be removed in a future version
	if(cb_onIntervalElapsed)(*cb_onIntervalElapsed)(temp, true, deviceIndex);

	if(_temperatures[deviceIndex] != rawTemp){
		_temperatures[deviceIndex] = rawTemp;
		//Invoked only if reading is valid. "valid" parameter will be removed in a future version
		if(cb_onTemperatureChange)(*cb_onTemperatureChange)(temp, true, deviceIndex);
	}

	#ifdef DEBUG_DS18B20
		Serial.print("DS18B20 (");
		Serial.print(deviceIndex);
		Serial.print("): ");
		Serial.print(temp);
		if(_unitsOM == unit_C) Serial.print(" °C");
		else Serial.print(" °F");
		Serial.println("");
	#endif
}

//==============================================================================================
//									PUBLIC
//==============================================================================================

void NonBlockingDallas::update(){
	switch(_currentState){
		case notFound:
			
		break;
		case waitingNextReading:
			waitNextReading();
		break;
		case waitingConversion:
			waitConversion();
		break;
		case readingSensor:
			readSensors();
		break;
	}
}

void NonBlockingDallas::requestTemperature(){
	_sensorsCount = _dallasTemp->getDeviceCount();
	if(_sensorsCount < 1) {
		#ifdef DEBUG_DS18B20
			Serial.print("DS18B20: no sensors found on the bus");
			Serial.println("");
		#endif
		
		return;
	}

	_currentState = waitingConversion;
	_startConversionMillis = millis();
	_dallasTemp->requestTemperatures();		//Requests a temperature conversion for all the sensors on the bus
	
	#ifdef DEBUG_DS18B20
		Serial.print("DS18B20: requested new reading");
		Serial.println("");
	#endif
}