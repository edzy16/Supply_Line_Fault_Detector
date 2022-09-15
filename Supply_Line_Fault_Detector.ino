#include "ZMPT101B.h"
#include "ACS712.h"
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"

/*
  This example shows how to measure the power consumption
  of devices in AC electrical system
*/

// We have ZMPT101B sensor connected to A0 pin of arduino
// Replace with your version if necessary
ZMPT101B voltageSensor(A0);

// We have 30 amps version sensor connected to A1 pin of arduino
// Replace with your version if necessary
const int analogIn = A1;
int mVperAmp = 66; // use 100 for 20A Module and 185 for 5A Module
int RawValue= 0;
int ACSoffset = 2500; 
double Voltage = 0;
double I = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

SoftwareSerial mySerial(9, 10); // setting 9 & 10 for serial ports. 9=TX , 10= RX

void setup()
{
  mySerial.begin(9600);//setting the baud rate of GSM Module
  Serial.begin(9600); // open the serial port at 9600 bps:
  // set up the LCD's number of columns and rows:
  lcd.init();
  lcd.backlight();
  pinMode(5, OUTPUT);
  // calibrate() method calibrates zero point of sensor,
  // It is not necessary, but may positively affect the accuracy
  // Ensure that no current flows through the sensor at this moment
  // If you are not sure that the current through the sensor will not leak during calibration - comment out this method
  //lcd.print("Calibrating... Ensure that no current flows through the sensor at this moment");
  digitalWrite(5, LOW);
  delay(100);
  voltageSensor.calibrate();
  
  //lcd.print("Done!");
}

  
void loop()
{
  // To measure voltage/current we need to know the frequency of voltage/current
  // By default 50Hz is used, but you can specify desired frequency
  // as first argument to getVoltageAC and getCurrentAC() method, if necessary

  float U = voltageSensor.getVoltageAC();
  RawValue = analogRead(analogIn);
  Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
  I = ((Voltage - ACSoffset) / mVperAmp);

  if (I<0){
    I = 0;
  }
  else{
    I=I*0.001;
  }
  U = (U*10)+90;
  if (U<200){
    U = 0;
    I = 0;
  }
 
  // To calculate the power we need voltage multiplied by current
  float P = U * I;
  
  Serial.println(String("U = ") + U + " V");
  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " Watts");
  lcd.setCursor(0, 0);
  lcd.println(String("P = ") + P + " Watts");
  
  delay(1000);
  
 if (U==0 || I==0) {
   digitalWrite(5,HIGH);
 }
 else{
   digitalWrite(5,LOW);
  }
 } 
