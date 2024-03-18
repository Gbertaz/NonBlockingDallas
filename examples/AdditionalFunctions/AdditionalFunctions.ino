/**
 * @brief Usage/Test of additional functions for lib NonBlockingDallas
 * 
 * @author oOpen <git@logisciel.com>
 * 
 * License: MIT
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

#define ONE_WIRE_BUS 2 // PIN of the Maxim DS18B20 temperature(s) sensor(s)
#define TIME_INTERVAL 1500 // Time interval among sensor readings [milliseconds]

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas temperatureSensors(&dallasTemp);

bool oneTimeFnCalled = false;

void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW)
{
  Serial.print(F("[NonBlockingDallas] handleTemperatureChange ==> deviceIndex="));
  Serial.print(deviceIndex);
  Serial.print(F(" | RAW="));
  Serial.print(temperatureRAW);
  Serial.print(F(" | "));
  Serial.print(temperatureSensors.rawToCelsius(temperatureRAW));
  Serial.print(F("°C | "));
  Serial.print(temperatureSensors.rawToFahrenheit(temperatureRAW));
  Serial.println(F("°F"));
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  temperatureSensors.begin(NonBlockingDallas::resolution_12, TIME_INTERVAL);
  temperatureSensors.onTemperatureChange(handleTemperatureChange);
  //temperatureSensors.requestTemperature();
}

void loop()
{
 temperatureSensors.update();

 // only call one time this function after tow seconds
 if(oneTimeFnCalled == false && millis() > 2000) {
  oneTimeFnCalled = true;
  testAdditionalsFunctions();
 }
}

void testAdditionalsFunctions()
{
  Serial.print(F("[NonBlockingDallas] : getDeviceCount() : "));
  Serial.println(temperatureSensors.getSensorsCount());
  Serial.println();

  // IF last index exist ?
  Serial.print(F("IF indexExist("));
  Serial.print(temperatureSensors.getSensorsCount() - 1);
  Serial.print(F(") ==> "));
  if (temperatureSensors.indexExist(temperatureSensors.getSensorsCount() - 1))
    Serial.println(F(" YES")); // YES if there is one sensor at least
  else
    Serial.println(F(" NO"));

  // IF last index +1 exist ?
  Serial.print(F("IF indexExist("));
  Serial.print(temperatureSensors.getSensorsCount());
  Serial.print(F(") ==> "));
  if (temperatureSensors.indexExist(temperatureSensors.getSensorsCount()))
    Serial.println(F(" YES"));
  else
    Serial.println(F(" NO")); // Always NO

  DeviceAddress deviceAddresses[temperatureSensors.getSensorsCount()];
  String stringAddresses[temperatureSensors.getSensorsCount()];

  for (size_t i = 0; i < temperatureSensors.getSensorsCount(); i++)
  {
    Serial.print(F("[Sensor index="));
    Serial.print(i);
    Serial.println(F("]"));

    Serial.print(F(" * Print all temperatures formats by using index | RAW="));
    Serial.print(temperatureSensors.getTemperatureRAW(i));
    Serial.print(F(" °C="));
    Serial.print(temperatureSensors.getTemperatureC(i));
    Serial.print(F(" °F="));
    Serial.println(temperatureSensors.getTemperatureF(i));

    Serial.println(F(" * Get DeviceAddress by using index & Store in deviceAddresses[]"));
    temperatureSensors.getDeviceAddress(i, deviceAddresses[i]);
    Serial.print(F(" * convertDeviceAddressToString(deviceAddresses["));
    Serial.print(i);
    Serial.print(F("]) ==> "));
    Serial.println(temperatureSensors.convertDeviceAddressToString(deviceAddresses[i]));

    Serial.println(F(" * Store address String representation in stringAddresses[]"));
    stringAddresses[i] = temperatureSensors.getAddressString(i);
    Serial.print(F(" * Sensor index="));
    Serial.print(i);
    Serial.print(F(" ==> "));
    Serial.println(stringAddresses[i]);

    Serial.println(F(" * Convert address String representation to DeviceAdress with convertDeviceAddressStringToDeviceAddress(stringAddresses[i], tmpAddress)"));
    DeviceAddress tmpAddress;
    temperatureSensors.convertDeviceAddressStringToDeviceAddress(stringAddresses[i], tmpAddress);
    Serial.print(F(" * get back converted to strig temperatureSensors.convertDeviceAddressToString(tmpAddress) ==> "));
    Serial.println(temperatureSensors.convertDeviceAddressToString(tmpAddress));
    Serial.print(F(" * Compare converted String with DeviceAddress using compareTowDeviceAddresses(tmpAddress, deviceAddresses[i]) ==> "));
    if (temperatureSensors.compareTowDeviceAddresses(tmpAddress, deviceAddresses[i]))
      Serial.println(F("Match !"));
    else
      Serial.println(F("Don't match !")); // never call here

    Serial.println();
  }

  /**
    * Validate range by DeviceAddress
    */

  Serial.print(F("validateAddressesRange(deviceAddresses, 2) ==> "));
  if (temperatureSensors.validateAddressesRange(deviceAddresses, 2))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  Serial.print(F("validateAddressesRange(deviceAddresses, 2, false) ==> "));
  if (temperatureSensors.validateAddressesRange(deviceAddresses, 2, false))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  // Add +1 fake/missing address
  DeviceAddress rangeInvalide[temperatureSensors.getSensorsCount() + 1];
  // copy your valide addresses
  memcpy(
      rangeInvalide,
      deviceAddresses,
      sizeof(deviceAddresses[0]) * temperatureSensors.getSensorsCount());
  // create a fake address
  for (size_t i = 0; i < 8; i++)
    rangeInvalide[temperatureSensors.getSensorsCount()][i] = i;

  Serial.print(F("validateAddressesRange(rangeInvalide, temperatureSensors.getSensorsCount()+1) ==> "));
  if (temperatureSensors.validateAddressesRange(rangeInvalide, temperatureSensors.getSensorsCount() + 1))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  Serial.print(F("validateAddressesRange(rangeInvalide, temperatureSensors.getSensorsCount()+1, false) ==> "));
  if (temperatureSensors.validateAddressesRange(rangeInvalide, temperatureSensors.getSensorsCount() + 1, false))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  /**
    * Validate range by string
    */

  Serial.print(F("validateAddressesRange(stringAddresses, temperatureSensors.getSensorsCount()) ==> "));
  if (temperatureSensors.validateAddressesRange(stringAddresses, temperatureSensors.getSensorsCount()))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  Serial.print(F("validateAddressesRange(stringAddresses, temperatureSensors.getSensorsCount(), false) ==> "));
  if (temperatureSensors.validateAddressesRange(stringAddresses, temperatureSensors.getSensorsCount(), false))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  // Add +1 fake/missing address
  String invalideStringAddresses[temperatureSensors.getSensorsCount() + 1];
  // copy your valide addresses
  for (size_t i = 0; i < temperatureSensors.getSensorsCount(); i++)
    invalideStringAddresses[i] = stringAddresses[i];
  // create a fake address
  invalideStringAddresses[temperatureSensors.getSensorsCount()] = "0123456789abcdef";

  Serial.print(F("validateAddressesRange(invalideStringAddresses, temperatureSensors.getSensorsCount()+1) ==> "));
  if (temperatureSensors.validateAddressesRange(invalideStringAddresses, temperatureSensors.getSensorsCount() + 1))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  Serial.print(F("validateAddressesRange(invalideStringAddresses, temperatureSensors.getSensorsCount()+1, false) ==> "));
  if (temperatureSensors.validateAddressesRange(invalideStringAddresses, temperatureSensors.getSensorsCount() + 1, false))
    Serial.println(F(" Yes"));
  else
    Serial.println(F(" NO"));

  /**
    * Map know sensors position index
    */

  int8_t mapPositionIndex[temperatureSensors.getSensorsCount()];
  Serial.print(F("mapIndexPositionOfDeviceAddressRange(deviceAddresses, temperatureSensors.getSensorsCount(), mapPositionIndex) ==> "));
  temperatureSensors.mapIndexPositionOfDeviceAddressRange(deviceAddresses, temperatureSensors.getSensorsCount(), mapPositionIndex);
  for (size_t i = 0; i < temperatureSensors.getSensorsCount(); i++)
  {
    Serial.print(F(" [index="));
    Serial.print(i);
    Serial.print(F(" position="));
    Serial.print(mapPositionIndex[i]);
    Serial.print(F("]"));
  }
  Serial.println();

  // use invalide range
  int8_t invalideMapPositionIndex[temperatureSensors.getSensorsCount() + 1];
  Serial.print(F("mapIndexPositionOfDeviceAddressRange(rangeInvalide, temperatureSensors.getSensorsCount()+1, invalideMapPositionIndex) ==> "));
  temperatureSensors.mapIndexPositionOfDeviceAddressRange(rangeInvalide, temperatureSensors.getSensorsCount() + 1, invalideMapPositionIndex);
  for (size_t i = 0; i < temperatureSensors.getSensorsCount() + 1; i++)
  {
    Serial.print(F(" [index="));
    Serial.print(i);
    Serial.print(F(" position="));
    Serial.print(invalideMapPositionIndex[i]);
    Serial.print(F("]"));
  }
  Serial.println();

  // make a reversed invalide range
  DeviceAddress reversedRangeInvalide[temperatureSensors.getSensorsCount() + 1];
  for (size_t i = 0; i < temperatureSensors.getSensorsCount() + 1; i++)
    for (size_t j = 0; j < 8; j++)
      reversedRangeInvalide[i][j] = rangeInvalide[temperatureSensors.getSensorsCount() - i][j];

  int8_t reversedInvalideMapPositionIndex[temperatureSensors.getSensorsCount() + 1];
  Serial.print(F("mapIndexPositionOfDeviceAddressRange(reversedRangeInvalide, temperatureSensors.getSensorsCount() + 1, reversedInvalideMapPositionIndex) ==> "));
  temperatureSensors.mapIndexPositionOfDeviceAddressRange(reversedRangeInvalide, temperatureSensors.getSensorsCount() + 1, reversedInvalideMapPositionIndex);
  for (size_t i = 0; i < temperatureSensors.getSensorsCount() + 1; i++)
  {
    Serial.print(F(" [index="));
    Serial.print(i);
    Serial.print(F(" position="));
    Serial.print(reversedInvalideMapPositionIndex[i]);
    Serial.print(F("]"));
  }
  Serial.println();
}
