#include "max6675.h"
#include <DHT.h>
#include <DHT_U.h>
#include <FirebaseArduino.h>
#include <NTPtimeESP.h>
#include <ESP8266WiFi.h>
#define DEBUG_ON

//Timestamp
NTPtime NTPch("ch.pool.ntp.org");
strDateTime dateTime;

//Wifi Settings
const char* WIFI_SSID = "BELL897";
const char* WIFI_PASSWORD = "923AF6F1";

//FireBase Settings
const char* FIREBASE_HOST = "coen390-52424.firebaseio.com";
const char* FIREBASE_AUTH = "rtsrO8xdfZjJybFKltxYTYItkxKDYhAum0h74XKD";

//Thermocouple Oven  
#define thermoDO1 D8
#define thermoCS1 D7
#define thermoCLK1 D6
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);

//DHT 22 sensors (temp+humidity)
#define DHTTYPE1    DHT22
#define DHTPIN1 D5

//Logic sensors
#define machine_sens1 D0

//RELAY
#define Switch_1 D1
#define Switch_2 D2
DHT_Unified dht1(DHTPIN1, DHTTYPE1);
uint32_t delayMS;


//Global Variables
//thermocouple temps
int temp1;
//dht22 temp
int temp2;
//dht22 humidity
int humidity1;
//Logic sensor (On/Off)
bool machine1;
bool OnOff;

//Database Paths
String GFS_Temp_Path = "machines/Big_Oven/Temperature";
String Powder_Temp_Path = "machines/Powder_Booth/Temperature";
String Powder_Hum_Path = "machines/Powder_Booth/Humidity";
String Powder_Status_Path = "machines/Big_Oven/machineStatus";
String Powder_Time_On_Path = "machines/Big_Oven/machineStatusTimeOn";
String Powder_Time_Off_Path = "machines/Big_Oven/machineStatusTimeOff";



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
OnOff = true;

pinMode(machine_sens1, INPUT);
machine1 = bool(digitalRead(machine_sens1));
pinMode(Switch_1, OUTPUT); digitalWrite(Switch_1, LOW);
pinMode(Switch_2, OUTPUT); digitalWrite(Switch_2, LOW);

sensor_t sensor1;
dht1.temperature().getSensor(&sensor1);
dht1.humidity().getSensor(&sensor1);

delayMS = sensor1.min_delay / 1000;
 
Serial.println("- - - We are leaving the setup loop - - - ");
// wait for MAX chip to stabilize

delay(1000);

}



void loop() {
 // No switches on this one for now getFirebase();
 //If we do get switches, call them Switch_3 & Switch_4
 //getFirebase();
  getsensordata();

}

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

void setValueFirebase(String path, int value){
  Firebase.setInt(path, value);
}


void getFirebase(){
     if(Firebase.getBool("Control_Switches/Switch_3"))
   {
      Serial.println("Switch_3 On");
      digitalWrite(Switch_1, HIGH);
   }
   else
   {
      Serial.println("Switch_3 Off");
      digitalWrite(Switch_1, LOW);
   }

   if(Firebase.getBool("Control_Switches/Switch_4"))
   {
      Serial.println("Switch_4 On");
      digitalWrite(Switch_2, HIGH);
   }
     else
   {
      Serial.println("Switch_4 Off");
      digitalWrite(Switch_2, LOW);
   }
    
 }


void getsensordata(){
  
 //get thermocoupe data
 if(isnan(thermocouple1.readCelsius())) {
    Serial.println("Error Reading Big Oven Temperature. ");
  }
  else  {
    temp1 = thermocouple1.readCelsius();
    Serial.print("Big Oven Temp: "); Serial.println(temp1);
    Firebase.setInt("machines/Big_Oven/Temperature", temp1);
   }


  //get dht data
 sensors_event_t event1;
 dht1.temperature().getEvent(&event1);
 
 if (isnan(event1.temperature)) {
    Serial.println(F("Error reading Powder Booth Temp"));
  }
  else{
    temp2 = event1.temperature;
    Serial.print(F("Powder Booth Temperature: ")); Serial.print(event1.temperature); Serial.println(F("°C"));
    Firebase.setInt("machines/Powder_Booth/Temperature", temp2);
    }
  
 dht1.humidity().getEvent(&event1);

 if (isnan(event1.relative_humidity)) {
    Serial.println(F("Error reading Powder Booth humidity!"));
 }
 else {
    humidity1 = event1.relative_humidity;
    Serial.print(F("Powder Booth Humidity: ")); Serial.print(event1.relative_humidity); Serial.println(F("%"));
    Firebase.setInt("machines/Powder_Booth/Humidity", humidity1);

  }
  
 //get machine data
 machine1 = bool(digitalRead(machine_sens1));
 Serial.print("Big Oven Status: ");
 dateTime = NTPch.getNTPtime(-5.0, 2);
 
 if(machine1 == true){
   Serial.println("On");
   const String path = "Machine/Big_Oven/Temperatures/"+dateTime.epochTime;
   Firebase.setInt(path, temp1);

   if(machine1 != OnOff){
    Firebase.setInt("machines/Big_Oven/machineStatusTimeOn", dateTime.epochTime);
    }

    
  }
  else{
    Serial.println("Off");
    if(machine1 != OnOff){
    Firebase.setInt("machines/Big_Oven/machineStatusTimeOff", dateTime.epochTime);
    }
    
  }
 Firebase.setBool("machines/Big_Oven/machineStatus", machine1);
 OnOff = machine1;

}
