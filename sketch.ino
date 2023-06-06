#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "DHTesp.h"
#include <ESP32Servo.h> 
#include <NTPClient.h>
#include "time.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* ntpServer = "pool.ntp.org";

const int DHT_PIN = 21;
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#define TFT_DC 2
#define TFT_CS 15
#define NUM_SERVOS 2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
DHTesp dhtSensor;
Servo myServo[2];
bool check = false;



void setup() {
  
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  myServo[0].attach(5);
  myServo[1].attach(12);
  WiFi.begin(ssid, password, 6);
  tft.begin();
  tft.setRotation(1);

  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.print("Dang khoi dong.");
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("\n\nConnecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    tft.print(".");
  }
  tft.print("\nOK! IP=");
  tft.println(WiFi.localIP());
  delay(300);
  tft.setTextColor(ILI9341_MAGENTA);
  tft.print("\n--Ngay moi tot lanh! <3--");
  tft.setTextColor(ILI9341_WHITE);
  delay(5000);
}
void Time(){
  
  struct tm* timeinfo;
  time_t now;
  time(&now);
  
  timeinfo = localtime(&now);
  tft.setTextColor(ILI9341_CYAN);
  tft.print(timeinfo, "%A, %B %d %Y");
  tft.setTextColor(ILI9341_WHITE);
}
String getPollution(){
  //ha noi
  const String url = "http://api.openweathermap.org/data/2.5/air_pollution?lat=21.0294498&lon=105.8544441&appid=aad4e9129abbb4eca4ca7a8ef803b193";
  // nghe an
  //const String url ="http://api.openweathermap.org/data/2.5/air_pollution?lat=18.679585&lon=105.681335&appid=aad4e9129abbb4eca4ca7a8ef803b193";
  //hcm
  //const String url ="http://api.openweathermap.org/data/2.5/air_pollution?lat=10.762622&lon=106.660172&appid=aad4e9129abbb4eca4ca7a8ef803b193"
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(url);
  http.GET();
  String result = http.getString();

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, result);

  // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "error";
  }
  String type = doc["list"][0]["main"]["aqi"].as<String>();
  http.end();
  if(type == "5"){
    return "Rat xau :< !";
  }
  if(type == "4"){
    return "Khong tot.";
  }
  if(type == "3"){
    return "Trung binh.";
  }
  if(type == "2"){
    return "Binh thuong.";
  }
  else{
    return "Tot :>";
  }


}

void getTempAndHum(){
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  Time();
  tft.print("\nChat luong khong khi:\n");
  if(getPollution() == "Rat xau :< !" || getPollution() == "Khong tot."){
    tft.setTextColor(ILI9341_RED);
  }
  if(getPollution() == "Trung binh."){
    tft.setTextColor(ILI9341_YELLOW);
  }
  if(getPollution() == "Binh thuong." || getPollution() == "Tot :>"){
    tft.setTextColor(ILI9341_GREEN);
  }

  tft.print(getPollution());
  tft.setTextColor(ILI9341_WHITE);
  delay(300);
  getPollution();
  tft.print("\nNhiet Do: " + String(data.temperature, 2) + "*C");
  tft.print("\nDo am: " + String(data.humidity, 1) + "%");
  tft.print("\nTinh trang mai che:");
  tft.print(check?"Dang mo ra.":"\nDa thu vao.");
  tft.setTextColor(ILI9341_GREEN);
  tft.print("\n-------------");
  tft.setTextColor(ILI9341_WHITE);

  delay(2000);
  if(data.temperature > 38){
    if(!check){

      CheNang();
      check =true;
    }
  }
  else{
    if(check){
      ThuCheNang();
      check= false;
    }
  }

  if(data.humidity < 50){
    TuoiNuoc();
  }
  else{
    tft.print("\nTat ca binh thuong.\nDoi them 10p");
  }
  delay(10000);//10p = 600000
}

void TuoiNuoc(){
  tft.print("\nDo am thap \nDang tuoi nuoc ...");

  myServo[0].write(90);
  delay(5000);
  myServo[0].write(0);
  delay(1000);
  tft.print("\nTuoi xong");
  tft.setTextColor(ILI9341_GREEN);
  tft.print("\n----------------------");
  tft.setTextColor(ILI9341_WHITE);
}

void CheNang(){
  tft.print("\nTroi nang.");
  tft.print("\nDang keo mai che ...");
  delay(1000);
  myServo[1].write(120);
  delay(1000);
  tft.print("\nChe xong.");
  tft.setTextColor(ILI9341_GREEN);
  tft.print("\n----------------------");
  tft.setTextColor(ILI9341_WHITE);
}
void ThuCheNang(){
  tft.print("\nTroi im.");
  tft.print("\nDang thu mai che ...");
  delay(1000);
  myServo[1].write(0);
  delay(1000);
  tft.print("\nThu xong.");
  tft.setTextColor(ILI9341_GREEN);
  tft.print("\n----------------------");
  tft.setTextColor(ILI9341_WHITE);
}

void loop() {
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  getTempAndHum();
  // put your main code here, to run repeatedly:
  delay(1000); // this speeds up the simulation
}
