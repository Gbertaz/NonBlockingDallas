
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
NonBlockingDallas sensorDs18b20(&dallasTemp);    //Create a new instance of the NonBlockingDallas class

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  //Initialize the sensor passing the resolution, unit of measure and reading interval [milliseconds]
  sensorDs18b20.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, TIME_INTERVAL);

  //Callbacks
  sensorDs18b20.onIntervalElapsed(handleIntervalElapsed);
  sensorDs18b20.onTemperatureChange(handleTemperatureChange);

  //Call the following function if you want to request the temperature without waiting for TIME_INTERVAL to elapse
  //The temperature value will then be provided with the onTemperatureChange callback when ready
  sensorDs18b20.requestTemperature();
}

void loop() {
  
 sensorDs18b20.update();
  
  /*
   *  EVEN THOUGH THE SENSOR CONVERSION TAKES UP TO 750ms
   *  THE NonBlockingDallas LIBRARY WAITS FOR THE CONVERSION
   *  WITHOUT BLOCKING THE loop AND CALLS THE CALLBACKS
   *  WHEN THE TEMPERATURE VALUE IS READY
   */ 
}

//Invoked at every sensor reading (TIME_INTERVAL milliseconds)
void handleIntervalElapsed(float temperature, bool valid, int deviceIndex){

  Serial.print("Sensor ");
  Serial.print(deviceIndex);
  Serial.print(" temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  /*
   *  DO SOME AMAZING STUFF WITH THE TEMPERATURE
   */
}

//Invoked ONLY when the temperature changes between two sensor readings
void handleTemperatureChange(float temperature, bool valid, int deviceIndex){

  //Serial.print("Sensor ");
  //Serial.print(deviceIndex);
  //Serial.print(" new temperature: ");
  //Serial.print(temperature);
  //Serial.println(" °C");
  
  /*
   *  DO SOME AMAZING STUFF WITH THE TEMPERATURE
   */
}
