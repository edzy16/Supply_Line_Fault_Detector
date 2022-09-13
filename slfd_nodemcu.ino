#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include "ZMPT101B.h"
#include "ACS712.h"
#include "LiquidCrystal.h"
#include "SoftwareSerial.h"
const String key = "key=AAAAjAcKy-c:APA91bEBWahXd9qEEowZNCwrAJcBC3Ezy_ByXF8rvjs9VavRmzkl8AB4D8pckwOhpAhuzjwcExX_Un-9DtCVsOoJCc80SsdXYtHtQcodCkZxZRSqwYggd25IpGG_5HUkwcXny0VjePdo";
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
bool okay = true;
// Insert Firebase project API Key
#define API_KEY "AIzaSyBO6P-dL2dJ6S578nHPHrJCvlzLcWqg5O0"
#define DATABASE_URL "https://supply-line-fault-detector-default-rtdb.firebaseio.com/" 

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "test@gmail.com"
#define USER_PASSWORD "12345678"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Initialize WiFi
void initWiFi() {
WiFiManager wm;
bool res;
res = wm.autoConnect("Supply Line");

// anonymous ap
if(!res) {
    Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
}

// We have ZMPT101B sensor connected to A0 pin of arduino
// Replace with your version if necessary
ZMPT101B voltageSensor(A0);

// We have 30 amps version sensor connected to A1 pin of arduino
// Replace with your version if necessary
const int analogIn = 14;
int mVperAmp = 66; // use 100 for 20A Module and 185 for 5A Module
int RawValue= 0;
int ACSoffset = 2500; 
double Voltage = 0;
double I = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(22,23,5,18,19,21);

SoftwareSerial mySerial(9, 10); // setting 9 & 10 for serial ports. 9=TX , 10= RX

void setup()
{
 Serial.begin(115200);  
  // Initialize WiFi
  initWiFi();
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;
  config.database_url = DATABASE_URL;
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();

  Serial.print("User UID: ");
  Serial.print(uid);
   //pinMode(led, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // calibrate() method calibrates zero point of sensor,
  // It is not necessary, but may positively affect the accuracy
  // Ensure that no current flows through the sensor at this moment
  // If you are not sure that the current through the sensor will not leak during calibration - comment out this method
  //lcd.print("Calibrating... Ensure that no current flows through the sensor at this moment");
  
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
  if (U<150){
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

if(okay)
{
  if (U==0 || I==0)  {
  okay=!okay;
  //digitalWrite(LED_BUILTIN, LOW);   
  Serial.println("not ok");
writeData("isOkay",false);
  sendNotification();
  
    }
  }
  if(!okay)
{
  if (!(U==0 || I==0))
  {
  okay=!okay;
  //digitalWrite(LED_BUILTIN, HIGH);   
  Serial.println("okay");
writeData("isOkay",true);

  
    }



} 
}

bool writeData(String path,bool stat)
{
  if (Firebase.RTDB.setInt(&fbdo, "Data/"+path+"/", stat)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      return true;
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      return false;
    }
    Serial.println("done."); 
  }
    int sendNotification()
{

  HTTPClient http;        
  http.begin("https://fcm.googleapis.com/fcm/send");
  http.addHeader("Authorization",key);
  http.addHeader("Content-Type","application/json");
  Serial.print("Sending notification ! ==> ");  
  String data = String("{\"to\":\"/topics/pushNotifications\",\"notification\":{\"title\":\"Fault!\",\"body\":\"Fault Found at Transformer!\"}}");
  int httpCode = http.POST(data);
  if(httpCode == HTTP_CODE_OK){
    Serial.println("OK");
  }else{
    Serial.println("Failed");
  }
  http.end();
  return httpCode;
  
}
