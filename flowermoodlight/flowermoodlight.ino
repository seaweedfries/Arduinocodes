// Guide: https://create.arduino.cc/projecthub/earthglitch/elpis-flower-mood-light-80e286?ref=part&ref_id=15858&offset=318

// libraries needed:
// WiFiNINA
// Firebase Arduino based on WiFiNINA

#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

#include <Config.h>
#include <Firebase.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include <Firebase_TCP_Client.h>
#include <WCS.h>

#define FIREBASE_HOST "week-8-d8270-default-rtdb.firebaseio.com" //define firebase credentials
#define FIREBASE_AUTH "Z5c1j34EwHpolbhWdokiA9XyoIJctDEam2ovrvXN"

FirebaseData firebaseData;
String path = "/Week_8";

// determine pins
const int LIGHT_SENSOR_PIN_1 = A0; // analog reading from pin A0: R
const int LIGHT_SENSOR_PIN_2 = A1; // analog reading from pin A1: G
const int LIGHT_SENSOR_PIN_3 = A2; // analog reading from pin A2: B
const int PIN_RED   = 9; // output to pin 9: R
const int PIN_GREEN = 7; // output to pin 7: G
const int PIN_BLUE  = 6; // output to pin 6: B
const int aT = 400; // threshold for light intensity

// variables will change:
int r = constrain(r, 0, 256); //constrain variable to 0 - 256
int g = constrain(g, 0, 256); //constrain variable to 0 - 256
int b = constrain(b, 0, 256); //constrain variable to 0 - 256
int R = 0;
int G = 0;
int B = 0;

// dummy variables for functions
int i1;

int getcolourint(String i){ //get rgb integers from firebase
  if (Firebase.getString(firebaseData, path + i))
    {
      if (firebaseData.dataType() == "string"){
        return firebaseData.stringData().toInt();
      }
    }
    else
    {
      Serial.println("ERROR: " + firebaseData.errorReason());
      Serial.println();
    }
}

bool getbool(String i){ //get bool from firebase
  if (Firebase.getString(firebaseData, path + i))
    {
      if (firebaseData.dataType() == "string"){
        if (firebaseData.stringData() == "true") {
          return true;
        }
        else if (firebaseData.stringData() == "false") {
          return false;
        }
        }
      }
    else
    {
      Serial.println("ERROR: " + firebaseData.errorReason());
      Serial.println();
    }
}

void setstring(String i, String k) { //send string to firebase
  if (Firebase.setString(firebaseData, path + i, k))
    {
    }
    else
    {
      Serial.println("ERROR : " + firebaseData.errorReason());
      Serial.println();
    }
}

void reset() { //only for setup to ensure original positions
  analogWrite(PIN_RED,   0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE,  0);
}

void setColour(int R1, int G1, int B1) { //set values to led
  analogWrite(PIN_RED,   R1);
  analogWrite(PIN_GREEN, G1);
  analogWrite(PIN_BLUE,  B1);
}

void custommode(){ //mode 1: custom
  r = getcolourint("/R_Value");
  g = getcolourint("/G_Value");
  b = getcolourint("/B_Value");
  setColour(r, g, b);
}

void automode() { //mode 2: auto
  int a0 = analogRead(LIGHT_SENSOR_PIN_1); // input from A0
  int a1 = analogRead(LIGHT_SENSOR_PIN_2); // input from A1
  int a2 = analogRead(LIGHT_SENSOR_PIN_3); // input from A2
  
  if (r >= 256 && g >= 256 && b >= 256) {
    r = 250;
    b = 250;
    g = 250;
    setstring("/R_Value", "250");
    setstring("/G_Value", "250");
    setstring("/B_Value", "250");
      
  }
  
  else if (a0 < aT || a1 < aT || a2 < aT) { //rate of colour change affected by intensity of light, or rather the lack of light
    if (a0 < aT && r < 256) {
      r += 7*((aT-a0)/100);
      setstring("/R_Value", String(r));
    }

    if (a1 < aT && g < 256) {
      g += 7*((aT-a1)/100);
      setstring("/G_Value", String(g));
    }

    if (a2 < aT && b < 256) {
      b += 7*((aT-a2)/100);
      setstring("/B_Value", String(b));
    }
    setColour(r, g, b);
    delay(50);
  }
  
  else { //decrease values when no change in input from sensors
    if (r > 0) {
      r -= 10 ;
      setstring("/R_Value", String(r));
    }
    if (g > 0) {
      g -= 10 ;
      setstring("/G_Value", String(g));
    }
    if (b > 0) {
      b -= 10 ;
      setstring("/B_Value", String(b));
    }
    setstring("/G_Value", String(false));
    setColour(r, g, b);
    delay(50);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);  
  pinMode(PIN_RED,   OUTPUT); //establish output
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    delay(10000); //wait 10 seconds for connection
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, SECRET_SSID, SECRET_PASS);
  Firebase.reconnectWiFi(true);

  reset(); //indicator ready to move to loop()
  delay(1000);
  setColour(256,0,0);
  delay(1000);
  setColour(0,256,0);
  delay(1000);
  setColour(0,0,256);
  delay(1000);
  reset();
  delay(5000); // wait 5 second
}

void loop() {  
  //if custommode or reset not called, loop automode
  if (getbool("/Auto") == true){
    automode();
  }
  
  else if (getbool("/Auto") == false){
    custommode();
  }
}
