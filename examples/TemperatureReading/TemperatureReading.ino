//======================================================================
//======================================================================
//  Program: TemperatureReading.ino
//
//  Description: this simple sketch demonstrates how to use the
//               NonBlockingDallas libray for reading the DS18B20 sensor
//               without blocking the main loop(). Supports multiple
//               sensors on the same ONE WIRE bus.
//               https://github.com/Gbertaz/NonBlockingDallas
//
//
//  License:
//
//  Copyright(c) 2021 Giovanni Bertazzoni <nottheworstdev@gmail.com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//
//======================================================================
//======================================================================

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>                  //Include the NonBlockingDallas library

#define ONE_WIRE_BUS 2                          //PIN of the Maxim DS18B20 temperature sensor
#define TIME_INTERVAL 1500                      //Time interval among sensor readings [milliseconds]

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas temperatureSensors(&dallasTemp);    //Create a new instance of the NonBlockingDallas class

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  //Initialize the sensor passing the resolution and reading interval [milliseconds]
  temperatureSensors.begin(NonBlockingDallas::resolution_12, TIME_INTERVAL);

  //Callbacks
  temperatureSensors.onIntervalElapsed(handleIntervalElapsed);
  temperatureSensors.onTemperatureChange(handleTemperatureChange);
  temperatureSensors.onDeviceDisconnected(handleDeviceDisconnected);
  
  //Call the following function whenever you want to request a new temperature reading without waiting for TIME_INTERVAL to elapse
  temperatureSensors.requestTemperature();
}

void loop()
{
  temperatureSensors.update();
  
  /*
   *  EVEN THOUGH THE SENSOR CONVERSION TAKES UP TO 750ms
   *  THE NonBlockingDallas LIBRARY WAITS FOR THE CONVERSION
   *  WITHOUT BLOCKING THE loop AND CALLS THE CALLBACKS
   *  WHEN THE TEMPERATURE VALUE IS READY
   */ 
}

//Invoked at every VALID sensor reading. "valid" parameter will be removed in a feature version
void handleIntervalElapsed(int deviceIndex, int32_t temperatureRAW)
{
  /* 
  Serial.print(F("[NonBlockingDallas] handleIntervalElapsed ==> deviceIndex="));
  Serial.print(deviceIndex);
  Serial.print(F(" | RAW="));
  Serial.print(temperatureRAW);
  Serial.print(F(" | "));
  Serial.print(temperatureSensors.rawToCelsius(temperatureRAW));
  Serial.print(F("°C | "));
  Serial.print(temperatureSensors.rawToFahrenheit(temperatureRAW));
  Serial.println(F("°F"));
 */

  /*
   *  DO SOME AMAZING STUFF WITH THE TEMPERATURE
   */
}

//Invoked ONLY when the temperature changes between two VALID sensor readings. "valid" parameter will be removed in a feature version
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
  
  /*
   *  DO SOME AMAZING STUFF WITH THE TEMPERATURE
   */
}

//Invoked when the sensor reading fails
void handleDeviceDisconnected(int deviceIndex)
{
  Serial.print(F("[NonBlockingDallas] handleDeviceDisconnected ==> deviceIndex="));
  Serial.print(deviceIndex);
  Serial.println(F(" disconnected."));
}
