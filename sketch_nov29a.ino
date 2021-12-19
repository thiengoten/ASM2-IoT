#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>

//#define WIFI_SSID "Inland_T4.2"
//#define WIFI_PASSWORD "inland402"
//#define WIFI_SSID "Tam Thu"
//#define WIFI_PASSWORD "15041965"

#define FIREBASE_HOST "esp8266t-feaf4-default-rtdb.asia-southeast1.firebasedatabase.app"

#define FIREBASE_AUTH "jNpxxctq3gYAeFQbltG3bVI3kl3QBONvu6edJjUr"

//Define FirebaseESP8266 data object
FirebaseData fbdo;

unsigned long sendDataPrevMillis = 0;
String path = "/IoT/Stream";

void printResult(FirebaseData &data);
int rainPin = A0;
int redLED = 13; //D7
int relay = 12; //D6
// you can adjust the threshold value
int thresholdValue = 800;

void setup(){
  pinMode(rainPin, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(relay, HIGH);
  Serial.begin(115200);
  delay(500);

  EEPROM.begin(512);
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.autoConnect("NodejsWifi");
  Serial.println("Connected......OK");
  //Blynk.begin(auth,WiFi.SSID().c_str(), WiFi.psk().c_str());

//  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//  Serial.print("Connecting to Wi-Fi");
//  while (WiFi.status() != WL_CONNECTED)
//  {
//    Serial.print(".");
//    delay(300);
//  }
//  Serial.println();
//  Serial.print("Connected with IP: ");
//  Serial.println(WiFi.localIP());
//  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(1024);

  if (!Firebase.beginStream(fbdo, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}   

void loop(){
  if (millis() - sendDataPrevMillis > 1000)
  {
     int sensorValue = analogRead(rainPin);
     int ledValue = digitalRead(redLED);
     String relayStatus = "";
     
     Serial.println(sensorValue);

     if (Firebase.getString(fbdo, path + "/pauseRelay/relayStatus"))
      {
        Serial.println("GET Relay status -------------------------");
        Serial.println("PASSED");
        Serial.print("VALUE: ");
        relayStatus = fbdo.stringData();
        Serial.println(relayStatus);
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }
      
      // Set distance value from sensor to firebase
      String ledStatus = ledValue ? "On" : "Off";
      if (Firebase.setString(fbdo, path + "/Data", String(sensorValue)+ ";" + ledStatus))
      {
        Serial.println("SET sensor and led status -------------------------");
        Serial.println("PASSED");
        Serial.print("VALUE: ");
        printResult(fbdo);
        Serial.println("------------------------------------");
        Serial.println();
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }

     if(relayStatus == "on") {
       Serial.println("Pause relay");
       digitalWrite(relay, HIGH);
       return;
     }
     
     if(sensorValue < thresholdValue){
       Serial.println("Doesn't need watering");
       digitalWrite(redLED,LOW);
       digitalWrite(relay, HIGH);
       delay(500);
     } 
     else{
       Serial.println("Time to water your plant");
       digitalWrite(redLED, HIGH);
       digitalWrite(relay, LOW);
       delay(500);
     }
     
  }
}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_FLOAT)
        Serial.println(jsonData.floatValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
               jsonData.typeNum == FirebaseJson::JSON_NULL ||
               jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
  else if (data.dataType() == "blob")
  {

    Serial.println();

    for (int i = 0; i < data.blobData().size(); i++)
    {
      if (i > 0 && i % 16 == 0)
        Serial.println();

      if (i < 16)
        Serial.print("0");

      Serial.print(data.blobData()[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  else if (data.dataType() == "file")
  {

    Serial.println();

    File file = data.fileStream();
    int i = 0;

    while (file.available())
    {
      if (i > 0 && i % 16 == 0)
        Serial.println();

      int v = file.read();

      if (v < 16)
        Serial.print("0");

      Serial.print(v, HEX);
      Serial.print(" ");
      i++;
    }
    Serial.println();
    file.close();
  }
  else
  {
    Serial.println(data.payload());
  }
}
