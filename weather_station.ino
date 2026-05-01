#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "secrets.h"


const int buttonMenuPin = 18;
bool lastButtonState = HIGH;

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define MAX_STATES 4
enum Infos {TEMPERATURE, HUMIDITY, CLOUDS, VISIBILITY};
enum Infos state = TEMPERATURE;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String jsonBuffer;

void displayText(const char* text){
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(WHITE);

  display.setCursor((display.width()/2) - ((strlen(text)*6)/2), 30);
  display.println(text);
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonMenuPin, INPUT_PULLUP);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}

void loop() {
  int buttonState = digitalRead(buttonMenuPin);
  const char* currentText = "";
  switch(state) {
    case TEMPERATURE:     currentText = "TEMPERATURE";     break;
    case HUMIDITY:    currentText = "HUMIDITY";    break;
    case CLOUDS: currentText = "CLOUDS"; break;
    case VISIBILITY:    currentText = "VISIBILITY";    break;
  }

  displayText(currentText);

  if (buttonState == LOW && lastButtonState == HIGH) {
    state = static_cast<Infos>((static_cast<int>(state) + 1) % MAX_STATES);
    delay(100);
    
  }
  lastButtonState = buttonState;


  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = String("http://api.openweathermap.org/data/2.5/weather?q=Paris,FR&APPID=") + OPENWEATHER_API_KEY;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      Serial.print("Temperature = ");
      Serial.println(myObject["main"]["temp"]);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  
  delay(50);
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}