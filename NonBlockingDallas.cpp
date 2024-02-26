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

NonBlockingDallas::NonBlockingDallas(DallasTemperature *dallasTemp)
{
	_dallasTemp = dallasTemp;
	_sensorsCount = 0;
	_lastReadingMillis = 0;
	_startConversionMillis = 0;
	_conversionMillis = 0;
	_currentState = notFound;
	cb_onIntervalElapsed = NULL;
	cb_onTemperatureChange = NULL;
	for (int i = 0; i < ONE_WIRE_MAX_DEV; i++)
		_temperatures[i] = DEVICE_DISCONNECTED_RAW;
}

void NonBlockingDallas::begin(resolution res, unsigned long tempInterval)
{
	_tempInterval = tempInterval;
	_currentState = notFound;
	_conversionMillis = 750 / (1 << (12 - (uint8_t)res)); // Rough calculation of sensors conversion time
	_dallasTemp->begin();
	delay(50);
	_dallasTemp->setWaitForConversion(false); // Avoid blocking the CPU waiting for the sensors conversion
	_sensorsCount = _dallasTemp->getDeviceCount();

	if (_sensorsCount > 0)
	{
		_currentState = waitingNextReading;
		_dallasTemp->setResolution((uint8_t)res);
		for (int i = 0; i < _sensorsCount; i++)
		{
			_dallasTemp->getAddress(_sensorAddresses[i], i);
		}
	}

	if ((_tempInterval < _conversionMillis) || (_tempInterval > 4294967295UL))
	{
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
	if (_sensorsCount > 0)
	{
		Serial.print("DS18B20: parasite power is ");
		if (_dallasTemp->isParasitePowerMode())
			Serial.print("ON");
		else
			Serial.print("OFF");
	}
	Serial.println("");
#endif
}

//==============================================================================================
//									PRIVATE
//==============================================================================================

void NonBlockingDallas::waitNextReading()
{
	if (_lastReadingMillis != 0 && (millis() - _lastReadingMillis < _tempInterval - _conversionMillis))
		return;
	requestTemperature();
}

void NonBlockingDallas::waitConversion()
{
	if (_dallasTemp->isConversionComplete())
	{
		// Save the actual sensor conversion time to precisely calculate the next reading time
		_conversionMillis = millis() - _startConversionMillis;
		_currentState = readingSensor;
	}
}

void NonBlockingDallas::readSensors()
{
	for (int i = 0; i < _sensorsCount; i++)
	{
		readTemperatures(i);
	}

	_lastReadingMillis = millis();
	_currentState = waitingNextReading;
}

void NonBlockingDallas::readTemperatures(int deviceIndex)
{
	int32_t rawTemp = _dallasTemp->getTemp(_sensorAddresses[deviceIndex]);

	if (rawTemp == DEVICE_DISCONNECTED_RAW)
	{
		if (cb_onDeviceDisconnected)
			(*cb_onDeviceDisconnected)(deviceIndex);
		return;
	}

	// Invoked only if reading is valid.
	if (cb_onIntervalElapsed)
		(*cb_onIntervalElapsed)(deviceIndex, rawTemp);

	if (_temperatures[deviceIndex] != rawTemp)
	{
		_temperatures[deviceIndex] = rawTemp;
		// Invoked only if reading is valid.
		if (cb_onTemperatureChange)
			(*cb_onTemperatureChange)(deviceIndex, rawTemp);
	}

#ifdef DEBUG_DS18B20
	Serial.print("DS18B20 (");
	Serial.print(deviceIndex);
	Serial.print("): RAW=");
	Serial.print(rawTemp);
	Serial.print(" | ");
	Serial.print(this->rawToCelsius(rawTemp));
	Serial.print("°C | ");
	Serial.print(this->rawToFahrenheit(rawTemp));
	Serial.println("°F");
#endif
}

//==============================================================================================
//									PUBLIC
//==============================================================================================

void NonBlockingDallas::update()
{
	switch (_currentState)
	{
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

void NonBlockingDallas::requestTemperature()
{
	_sensorsCount = _dallasTemp->getDeviceCount();
	if (_sensorsCount < 1)
	{
#ifdef DEBUG_DS18B20
		Serial.print("DS18B20: no sensors found on the bus");
		Serial.println("");
#endif

		return;
	}

	_currentState = waitingConversion;
	_startConversionMillis = millis();
	_dallasTemp->requestTemperatures(); // Requests a temperature conversion for all the sensors on the bus

#ifdef DEBUG_DS18B20
	Serial.print("DS18B20: requested new reading");
	Serial.println("");
#endif
}

/**
 * @brief Functions below are extensions to the origninal NonBlockingDallas
 
 * @author oOpen <git@logisciel.com>
 *
 * Litle breaking changes :
 * --> Change in function begin() : remove units parameters.
 * --> In usage of callback's functions : arguments are not the sames.
 */

/**
 * Functions below get by deviceIndex
 */

/**
 * @brief Get the number of sensors found
 *
 * @return uint8_t Number of sensors found
 */
uint8_t NonBlockingDallas::getSensorsCount()
{
	return this->_sensorsCount;
}

/**
 * @brief Validate a sensor index
 *
 * @return true if index exist
 * @return false if not exist
 */
bool NonBlockingDallas::indexExist(uint8_t deviceIndex)
{
	if (this->_sensorsCount > 0 && deviceIndex >= 0 && deviceIndex < this->_sensorsCount)
		return true;
	return false;
}

/**
 * @brief If exist index, copie address into DeviceAddress
 *
 * @return true if index exist
 * @return false else index not exist
 */
bool NonBlockingDallas::getDeviceAddress(uint8_t deviceIndex, DeviceAddress deviceAddress)
{
	if (deviceIndex >= 0 && deviceIndex < this->_sensorsCount)
	{
		for (size_t i = 0; i < 8; i++)
			deviceAddress[i] = this->_sensorAddresses[deviceIndex][i];
		return true;
	}
	return false;
}

/**
 * @brief Get address String representation from index
 *
 * @return String representation of the address
 */
String NonBlockingDallas::getAddressString(uint8_t deviceIndex)
{
	DeviceAddress tmpAddress;
	this->getDeviceAddress(deviceIndex, tmpAddress);
	return this->convertDeviceAddressToString(tmpAddress);
}

/**
 * @brief Get RAW Temperature from sensor index
 *
 * @return int32_t temperature IF index exist
 * @return DEVICE_DISCONNECTED_RAW if index not exist
 */
int32_t NonBlockingDallas::getTemperatureRAW(uint8_t deviceIndex)
{
	if (this->indexExist(deviceIndex))
		return this->_temperatures[deviceIndex];
	else
		return DEVICE_DISCONNECTED_RAW;
}

/**
 * @brief Get Temperature from sensor index
 *
 * @return float temperature IF index exist
 * @return DEVICE_DISCONNECTED_C if index not exist
 */
float NonBlockingDallas::getTemperatureC(uint8_t deviceIndex)
{
	if (this->indexExist(deviceIndex))
		return this->rawToCelsius(this->_temperatures[deviceIndex]);
	else
		return DEVICE_DISCONNECTED_C;
}

/**
 * @brief Get Temperature from sensor index
 *
 * @return float temperature IF index exist
 * @return DEVICE_DISCONNECTED_F if index not exist
 */
float NonBlockingDallas::getTemperatureF(uint8_t deviceIndex)
{
	if (this->indexExist(deviceIndex))
		return this->rawToFahrenheit(this->_temperatures[deviceIndex]);
	else
		return DEVICE_DISCONNECTED_F;
}

/**
 * Functions below get by DeviceAddress
 */

/**
 * @brief Get device index from DeviceAddress
 *
 * @return  x position index : 0 <= x < _sensorsCount
 * @return -1 address not found
 * @return -2 bus have no sensor
 */
int8_t NonBlockingDallas::getIndex(DeviceAddress deviceAddress)
{
	if (this->_sensorsCount > 0)
	{
		for (size_t i = 0; i < this->_sensorsCount; i++)
			if (this->compareTowDeviceAddresses(this->_sensorAddresses[i], deviceAddress))
				return i;
		return -1;
	}
	else
	{
		return -2;
	}
}

/**
 * @brief Get RAW Temperature from sensor DeviceAddress
 *
 * @return int32_t temperature IF DeviceAddress exist
 * @return DEVICE_DISCONNECTED_RAW if DeviceAddress not exist
 */
int32_t NonBlockingDallas::getTemperatureRAW(DeviceAddress deviceAddress)
{
	uint8_t tempIndex = this->getIndex(deviceAddress);
	if (tempIndex >= 0)
		return this->_temperatures[tempIndex];
	else
		return DEVICE_DISCONNECTED_RAW;
}

/**
 * @brief Get Temperature from sensor DeviceAddress
 *
 * @return float temperature IF DeviceAddress exist
 * @return DEVICE_DISCONNECTED_C if DeviceAddress not exist
 */
float NonBlockingDallas::getTemperatureC(DeviceAddress deviceAddress)
{
	uint8_t tempIndex = this->getIndex(deviceAddress);
	if (tempIndex >= 0)
		return this->rawToCelsius(this->_temperatures[tempIndex]);
	else
		return DEVICE_DISCONNECTED_C;
}

/**
 * @brief Get Temperature from sensor DeviceAddress
 *
 * @return float temperature IF DeviceAddress exist
 * @return DEVICE_DISCONNECTED_F if DeviceAddress not exist
 */
float NonBlockingDallas::getTemperatureF(DeviceAddress deviceAddress)
{
	uint8_t tempIndex = this->getIndex(deviceAddress);
	if (tempIndex >= 0)
		return this->rawToCelsius(this->_temperatures[tempIndex]);
	else
		return DEVICE_DISCONNECTED_F;
}

/**
 * Functions below get by String address representation
 */

/**
 * @brief Get device index from address String representation
 *
 * @return  x position index : 0 <= x < _sensorsCount
 * @return -1 address not found
 * @return -2 bus have no sensor
 */
int8_t NonBlockingDallas::getIndex(String addressString)
{
	DeviceAddress tmpAddress;
	this->convertDeviceAddressStringToDeviceAddress(addressString, tmpAddress);
	return this->getIndex(tmpAddress);
}

/**
 * @brief Get RAW Temperature from sensor address String representation
 *
 * @return int32_t temperature IF address String representation exist
 * @return DEVICE_DISCONNECTED_RAW if address String representation not exist
 */
int32_t NonBlockingDallas::getTemperatureRAW(String addressString)
{
	uint8_t tempIndex = this->getIndex(addressString);
	if (tempIndex >= 0)
		return this->_temperatures[tempIndex];
	else
		return DEVICE_DISCONNECTED_RAW;
}

/**
 * @brief Get Temperature from sensor address String representation
 *
 * @return float temperature IF address String representation exist
 * @return DEVICE_DISCONNECTED_C if address String representation not exist
 */
float NonBlockingDallas::getTemperatureC(String addressString)
{
	uint8_t tempIndex = this->getIndex(addressString);
	if (tempIndex >= 0)
		return this->rawToCelsius(this->_temperatures[tempIndex]);
	else
		return DEVICE_DISCONNECTED_C;
}

/**
 * @brief Get Temperature from sensor address String representation
 *
 * @return float temperature IF address String representation exist
 * @return DEVICE_DISCONNECTED_F if address String representation not exist
 */
float NonBlockingDallas::getTemperatureF(String addressString)
{
	uint8_t tempIndex = this->getIndex(addressString);
	if (tempIndex >= 0)
		return this->rawToCelsius(this->_temperatures[tempIndex]);
	else
		return DEVICE_DISCONNECTED_F;
}

/**
 * Functions below are helpers
 */

/**
 * @brief Comprare tow DeviceAddress
 *
 * @return true if addresses are the same
 * @return false if not the same
 */
bool NonBlockingDallas::compareTowDeviceAddresses(DeviceAddress deviceAddress1, DeviceAddress deviceAddress2)
{
	for (size_t i = 0; i < 8; i++)
		if (deviceAddress1[i] != deviceAddress2[i])
			return false;
	return true;
}

/**
 * @brief Convert a DeviceAddress to address String representation
 *
 * @return String representation of the address
 */
String NonBlockingDallas::convertDeviceAddressToString(DeviceAddress deviceAddress)
{
	String addressString = "";
	for (size_t i = 0; i < 8; i++)
	{
		if (deviceAddress[i] < 16)
			addressString += "0";
		addressString += String(deviceAddress[i], HEX);
	}
	return addressString;
}

/**
 * @brief Convert a String address representation to a DeviceAddress
 *
 * @return true if string lenght is good
 * @return false if string lenght is wrong
 */
bool NonBlockingDallas::convertDeviceAddressStringToDeviceAddress(String addressString, DeviceAddress deviceAddress)
{
	if (addressString.length() == 16)
	{
		for (size_t i = 0; i < 8; i++)
		{
			char cMSB = addressString[2 * i];
			char cLSB = addressString[2 * i + 1];
			uint8_t tmpValue = 0;
			if (towCharToHex(cMSB, cLSB, &tmpValue))
				deviceAddress[i] = tmpValue;
			else
				return false;
		}
		return true;
	}
	return false;
}

/**
 * @brief Convert a RAW Temperature to °C (celsisus)
 *
 * @return float temperature in °C
 */
float NonBlockingDallas::rawToCelsius(int32_t rawTemperature)
{
	return (float)rawTemperature * 0.0078125f;
}

/**
 * @brief Convert a RAW Temperature to °F (fahrenheit)
 *
 * @return float temperature in °F
 */
float NonBlockingDallas::rawToFahrenheit(int32_t rawTemperature)
{
	return ((float)rawTemperature * 0.0140625f) + 32.0f;
}

/**
 * @brief Validate a DeviceAdress[] range
 *
 * @param exclusiveListSet IF set to true, bus can't not have others devices than there listed
 *
 * @return true if all devices are finds in the bus
 * @return false if not all devices are finds in the bus
 */
bool NonBlockingDallas::validateAddressesRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, bool exclusiveListSet)
{
	// Exclusif mode, number of device must be equal
	if (exclusiveListSet && (numberOfAddresses != this->_sensorsCount))
		return false;

	// check if we find all the devices expected
	for (size_t i = 0; i < numberOfAddresses; i++)
	{
		if (this->getIndex(addressesRangeToValidate[i]) < 0)
			return false;
	}
	return true;
}

/**
 * @brief Validate a String addresses [] range
 *
 * @param exclusiveListSet IF set to true, bus can't not have others devices than there listed
 *
 * @return true if all devices are finds in the bus
 * @return false if not all devices are finds in the bus
 */
bool NonBlockingDallas::validateAddressesRange(String addressesStrings[], uint8_t numberOfAddresses, bool exclusiveListSet)
{
	DeviceAddress tmpAddresses[numberOfAddresses];
	for (size_t i = 0; i < numberOfAddresses; i++)
		this->convertDeviceAddressStringToDeviceAddress(addressesStrings[i], tmpAddresses[i]);

	return this->validateAddressesRange(tmpAddresses, numberOfAddresses, exclusiveListSet);
}

/**
 * @brief Convert a suposed 'HEX char' into a uint
 *
 * @return uint8_t HEX Numbrer
 * @return 255 if char is not valide
 */
uint8_t NonBlockingDallas::charToHex(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'A') && (c <= 'F'))
		return 10 + c - 'A';
	if ((c >= 'a') && (c <= 'f'))
		return 10 + c - 'a';
	return 255;
}

/**
 * @brief Convert a tow supposed 'HEX char' into a uint
 *
 * @return true if conversion work
 * @return false if conversion fail
 */
bool NonBlockingDallas::towCharToHex(char MSB, char LSB, uint8_t *ptrValue)
{
	uint8_t uMSB = charToHex(MSB);
	uint8_t uLSB = charToHex(LSB);

	if (uMSB == 255 || uLSB == 255)
		return false;

	if (uMSB > 0)
		*ptrValue = uMSB * 16 + uLSB;
	else
		*ptrValue = uLSB;

	return true;
}

/**
 * @brief Map index positions of a DeviceAddress range[] into a int8_t Array
 *
 * x >= 0 if sensor is found
 * x = -1 if not found
 * x = -2 if no sensor at all is detected
 */
void NonBlockingDallas::mapIndexPositionOfDeviceAddressRange(DeviceAddress addressesRangeToValidate[], uint8_t numberOfAddresses, int8_t mapedPositions[])
{
	for (size_t i = 0; i < numberOfAddresses; i++)
		mapedPositions[i] = this->getIndex(addressesRangeToValidate[i]);
}
