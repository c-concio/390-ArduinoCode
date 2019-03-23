//390-Arduino Code - Liquid Side
//Chris Torossian

#include "max6675.h"
#include <DHT.h>
#include <DHT_U.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#define DEBUG_ON


//-----------Wifi Settings
const char* WIFI_SSID = "BELL897";
const char* WIFI_PASSWORD = "923AF6F1";

//----------FireBase Settings
const char* FIREBASE_HOST = "coen390-52424.firebaseio.com";
const char* FIREBASE_AUTH = "rtsrO8xdfZjJybFKltxYTYItkxKDYhAum0h74XKD";

//----------Thermocouple Oven  
#define thermoDO1 D8
#define thermoCS1 D7
#define thermoCLK1 D6
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);

//------------DHT 22 sensors (temp+humidity)
#define DHTTYPE1    DHT22
#define DHTPIN1 D1

//------------Logic sensors
#define machine_sens1 D0

//-----------RELAY
#define Switch_1 D2
#define Switch_2 D3
DHT_Unified dht1(DHTPIN1, DHTTYPE1);
uint32_t delayMS;


//----------------------------Global Variables
//---------thermocouple temps
int temp1;
//----------dht22 temp
int temp2;
//----------dht22 humidity
int humidity1;
//----------Logic sensor (On/Off)
bool machine1;

//-----------Database Paths
String GFS_Temp_Path = "machines/GFS_Oven/temperature";
String Liquid_Temp_Path = "machines/Liquid_Booth/temperature";
String Liquid_Hum_Path = "machines/Liquid_Booth/humidity";
String GFS_Status_Path = "machines/GFS_Oven/machineStatus";


//---------------------------------Setup-----------------------------------//

void setup() {
  
Serial.begin(9600);

Serial.println("- - - System On Bitches - - -");
Serial.println("- - - We are in the setup loop - - -");

connectWiFi();

Serial.println("Connecting to Firebase ");
Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

dht1.begin();

temp1 = 0;
temp2 = 0;
humidity1 = 0;
pinMode(machine_sens1, INPUT);
machine1 = bool(digitalRead(machine_sens1));
pinMode(Switch_1, OUTPUT); digitalWrite(Switch_1, LOW);
pinMode(Switch_2, OUTPUT); digitalWrite(Switch_2, LOW);
//Using this output as a power source for the DHT-22 Sensor
pinMode(D5, OUTPUT); digitalWrite(D5, HIGH);

sensor_t sensor1;
dht1.temperature().getSensor(&sensor1);
dht1.humidity().getSensor(&sensor1);
 
Serial.println("- - - We are leaving the setup loop - - - ");
// wait for MAX chip to stabilize

delay(1000);

}

//------------------------Loop--------------------------------//

void loop() {
 // No switches on this one for now getFirebase();
 //If we do get switches, call them Switch_3 & Switch_4
  getFirebase();
  getsensordata();

}

//------------------------Connect Wifi--------------------------------//

  void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
}

//-------------------------SetValueToDatabse---SetBoolFirebase----------------------------//

void setValueFirebase(String path, int value){
  int trial = 1; 
  Firebase.set(path, value);
  delay(500);
  while(Firebase.failed() && trial < 4){
    Serial.print("Error Setting value ");
    Serial.println(path);
    Firebase.set(path, value);
    trial++;
    }
}

void setBoolFirebase(String path, bool value){
  int trial = 1; 
  Firebase.setBool(path, value);
  delay(500);
  while(Firebase.failed() && trial < 4){
    Serial.print("Error Setting value ");
    Serial.println(path);
    Firebase.set(path, value);
    trial++;
    }
}


//------------------------getFirebaseData--------------------------------//

void getFirebase(){
     if(Firebase.getBool("Control_Switches/Switch_1"))
   {
      Serial.println("Switch_1 On");
      digitalWrite(Switch_1, HIGH);
   }
   else
   {
      Serial.println("Switch_1 Off");
      digitalWrite(Switch_1, LOW);
   }

   if(Firebase.getBool("Control_Switches/Switch_2"))
   {
      Serial.println("Switch_2 On");
      digitalWrite(Switch_2, HIGH);
   }
     else
   {
      Serial.println("Switch_2 Off");
      digitalWrite(Switch_2, LOW);
   }
    
 }

//-----------------------------GetSensorData---------------------------//

void getsensordata(){
  delay(200);
 
 //------------------get thermocoupe data
 
 if(isnan(thermocouple1.readCelsius())) {
    Serial.println("Error Reading GFS Oven Temp");
  }
  else  {
    temp1 = thermocouple1.readCelsius();
    Serial.print("GFS_Oven Temp : "); Serial.println(temp1);
    setValueFirebase(GFS_Temp_Path, temp1);
   }


  //---------------get dht data
 sensors_event_t event1;
 dht1.temperature().getEvent(&event1);
 
 if (isnan(event1.temperature)) {
    Serial.println(F("Error reading Liquid Booth Temp."));
  }
  else{
    temp2 = event1.temperature;
    Serial.print(F("Liquid Paint Booth Temperature: ")); Serial.print(event1.temperature); Serial.println(F("°C"));
    setValueFirebase(Liquid_Temp_Path, temp2);
    }
  
 dht1.humidity().getEvent(&event1);

 if (isnan(event1.relative_humidity)) {
    Serial.println(F("Error reading Liquid Booth humidity!"));
 }
 else {
    humidity1 = event1.relative_humidity;
    Serial.print(F("Liquid Booth Humidity: ")); Serial.print(event1.relative_humidity); Serial.println(F("%"));
    setValueFirebase(Liquid_Hum_Path, humidity1);
  }
  
 //--------------------get machine data
 
 machine1 = bool(digitalRead(machine_sens1));
 Serial.print("GFS Oven Status: ");
 
 if(machine1 == true){
   Serial.println("On");
   setBoolFirebase(GFS_Status_Path, true);
  }
  else{
    Serial.println("Off");
    setBoolFirebase(GFS_Status_Path, false);
  }
}
