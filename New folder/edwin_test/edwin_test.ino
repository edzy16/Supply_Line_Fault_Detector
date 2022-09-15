#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
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
  
  Firebase.begin(&config, &auth);
  pinMode(35,INPUT);
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
  
  delay(100);

}

  
void loop()
{
if(okay)
{
  if (digitalRead(35))  {
  okay=!okay;
  //digitalWrite(LED_BUILTIN, LOW);   
  Serial.println("not ok");
  writeData("isOkay",false);
  sendNotification();
  
    }
  }
  if(!okay)
{
  if (!(digitalRead(35)))
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
