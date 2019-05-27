// CS 580 - Wearable Computing
// 09.08.2016


#include "CurieIMU.h"
#include "CurieBLE.h"

int inPin = 2;         // the number of the input (button) pin
int outPin = 13;       // the number of the output (led) pin

int state = LOW;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

unsigned long time = 0;         // the last time the output pin was toggled
unsigned long duration = 0;   // total duration
unsigned long calories = 0;   // total calories burnt
int stepCount = 0;    // total step count

int conversionFactor = 0.045;

BLEPeripheral blePeripheral;  // create a new peripheral instance
BLEService bleService("");  // create a new service
// create led characteristic and allow remote device to read
BLECharCharacteristic stepCountCharacteristic("", BLERead);
//create button characteristic and allow remote device to write
BLECharCharacteristic buttonCharacteristic("", BLEWrite);


void setup()
{
  while (!Serial) { ;}
    Serial.begin(9600);
  
  pinMode(inPin, INPUT);
  pinMode(outPin, OUTPUT);

  // set the local name peripheral advertises
  blePeripheral.setLocalName("MyButton");
  // set the UUID for the service this peripheral advertises
  blePeripheral.setAdvertisedServiceUuid(bleService.uuid());

  // add services and the characteristics
  blePeripheral.addAttribute(bleService);
  blePeripheral.addAttribute(stepCountCharacteristic);
  blePeripheral.addAttribute(buttonCharacteristic);

  // initialize the values for characteristics
  stepCountCharacteristic.setValue(0);
  buttonCharacteristic.setValue(0);

  // advertise the service
  blePeripheral.begin();

  Serial.println("Bluetooth device started to advertise... you can make connection now.");

  CurieIMU.begin();   // intialize the sensor
  // turn on step detection mode:
  CurieIMU.setStepDetectionMode(CURIE_IMU_STEP_MODE_NORMAL);
  CurieIMU.setStepCountEnabled(true);   // enable step counting
  CurieIMU.interrupts(CURIE_IMU_STEP);  // turn on step detection

  Serial.println("Click button to start the pedometer\n");
}

void loop()
{

  blePeripheral.poll(); // poll the BLE peripheral
  
  reading = buttonCharacteristic.value();  

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading == HIGH && previous == LOW) {
    Serial.print("Switch ON\n");
    state = HIGH;
  }
  if (reading == LOW && previous == HIGH) {  // if switch is off
    Serial.print("Switch OFF\n");
    state = LOW;
    // calculate duration by subtracting the time (switch was on)
    // from the current time (switch is off):
    duration = millis() - time;                                      
    calories = stepCount * conversionFactor;   // calculate total calories burnt
    Serial.print("Step count: "); Serial.println(stepCount);
    Serial.print("Duration: "); Serial.println(duration);
    Serial.print("Calories burnt: "); Serial.println(calories);
    stepCount = 0;
    }    
  time = millis();    
 

  // if switch is on, LED lights up each time 
  // when a step is detected and then go off
  if (state == HIGH) {
    if (CurieIMU.stepsDetected()) {
      digitalWrite(outPin, HIGH);
      delay(100);
      digitalWrite(outPin, LOW);
      stepCount++;  // increase step count when a step is detected
      stepCountCharacteristic.setValue(stepCount);
    }
  }
  previous = reading;
}

