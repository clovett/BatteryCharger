#include <Arduino.h>

int batteryCapacity = 2500;    // capacity rating of battery in mAh
float resistance = 10.0;       // measured resistance of the power resistor
int cutoffVoltage = 1600;      // maximum battery voltage (in mV) that should not be exceeded
float cutoffTemperatureC = 35; // maximum battery temperature that should not be exceeded (in degrees C)
unsigned long cutoffTime = 46800000; // maximum charge time of 13 hours that should not be exceeded

int outputPin = 9;     // Output signal wire connected to digital pin 9
int outputValue = 150; // value of PWM output signal

int analogPinOne = 0;      // first voltage probe connected to analog pin 1
float valueProbeOne = 0;   // variable to store the value of analogPinOne
float voltageProbeOne = 0; // calculated voltage at analogPinOne

int analogPinTwo = 1;      // second voltage probe connected to analog pin 2
float valueProbeTwo = 0;   // variable to store the value of analogPinTwo
float voltageProbeTwo = 0; // calculated voltage at analogPinTwo

int analogPinThree = 2;    // third voltage probe connected to analog pin 2
float valueProbeThree = 0; // variable to store the value of analogPinThree
float tmp36Voltage = 0;    // calculated voltage at analogPinThree
float temperatureC = 0;    // calculated temperature of probe in degrees C
// float temperatureF = 0;     //calculated temperature of probe in degrees F

float voltageDifference = 0;                // difference in voltage between analogPinOne and analogPinTwo
float batteryVoltage = 0;                   // calculated voltage of battery
float current = 0;                          // calculated current through the load (in mA)
float targetCurrent = batteryCapacity / 10; // target output current (in mA) set at C/10 or 1/10 of the battery capacity per hour
float currentError = 0;                     // difference between target current and actual current (in mA)

bool comPort = true;

void printPrompt(String prompt)
{
  if (comPort)
  {
    Serial.print(prompt);
  }
}

void printMessage(String prompt, String value)
{
  if (comPort)
  {
    Serial.print(prompt);
    Serial.println(value);
  }
}

void setup()
{
  if (comPort)
  {
    Serial.begin(115200); //  setup serial
  }
  pinMode(outputPin, OUTPUT); // sets the pin as output

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

float readProbeOne(){
  valueProbeOne = analogRead(analogPinOne);        // read the input value at probe one
  return (valueProbeOne * 5000.0) / 1023.0; // calculate voltage at probe one in milliVolts
}

float readProbeTwo(){
  valueProbeTwo = analogRead(analogPinTwo);        // read the input value at probe two
  return (valueProbeTwo * 5000.0) / 1023.0; // calculate voltage at probe two in milliVolts
}

float readTemperature(){
  valueProbeThree = analogRead(analogPinThree); // read the input value at probe three
  tmp36Voltage = valueProbeThree * 5.0;         // converting that reading to voltage
  tmp36Voltage /= 1024.0;
  return (tmp36Voltage - 0.5) * 100; // converting from 10 mv per degree wit 500 mV offset to degrees ((voltage - 500mV) times 100)
}


void loop()
{
  analogWrite(outputPin, outputValue); // Write output value to output pin

  printMessage("Output: ", String(outputValue)); // display output values for monitoring with a computer

  voltageProbeOne = 0;
  voltageProbeTwo = 0;
  temperatureC = 0;

  // average 10 readings to ensure we have smooth values.
  printPrompt("Reading...");
  for (int i = 0; i < 10; i++) {
    delay(1000); // delay 1 second between each read operation
    voltageProbeOne += readProbeOne();
    voltageProbeTwo += readProbeTwo(); 
    temperatureC += readTemperature();
  }
  printMessage(".", "");

  voltageProbeOne /= 10;
  voltageProbeTwo /= 10; 
  temperatureC /= 10;

  // display voltage at probe one
  printMessage("Voltage Probe One (mV): ", String(voltageProbeOne));

  // display voltage at probe two
  printMessage("Voltage Probe Two (mV): ", String(voltageProbeTwo));

  batteryVoltage = 5000 - voltageProbeTwo; // calculate battery voltage
  // display battery voltage
  printMessage("Battery Voltage (mV): ", String(batteryVoltage));

  current = (voltageProbeTwo - voltageProbeOne) / resistance; // calculate charge current
   // display target current
  printMessage("Target Current (mA): ", String(targetCurrent));
  // display actual current
  printMessage("Battery Current (mA): ", String(current));

  currentError = targetCurrent - current; // difference between target current and measured current
  // display current error
  printMessage("Current Error  (mA): ", String(currentError));

  // display the temperature in degrees C
  printMessage("Temperature (degrees C) ", String(temperatureC));

  /*
   temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;     //convert to Fahrenheit
   printMessage("Temperature (degrees F) ");
   String(temperatureF);
  */

  if (abs(currentError) > 10) // if output error is large enough, adjust output
  {
    outputValue = outputValue + currentError / 10;

    if (outputValue < 1) // output can never go below 0
    {
      outputValue = 0;
    }

    if (outputValue > 254) // output can never go above 255
    {
      outputValue = 255;
    }
    analogWrite(outputPin, outputValue); // write the new output value
  }

  if (temperatureC > cutoffTemperatureC) // stop charging if the battery temperature exceeds the safety threshold
  {
    outputValue = 0;

    digitalWrite(LED_BUILTIN, LOW);
    printMessage("Max Temperature Exceeded", "");
  }

  if (batteryVoltage > cutoffVoltage) // stop charging if the battery voltage exceeds the safety threshold
  {
    outputValue = 0;
    printMessage("Max Voltage Exceeded", "");
  }

  if (millis() > cutoffTime) // stop charging if the charge time threshold
  {
    outputValue = 0;
    printMessage("Max Charge Time Exceeded", "");
  }

  if (outputValue > 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  printMessage("", ""); // extra spaces to make debugging data easier to read

  delay(1000); // delay 1 second before next iteration
}
