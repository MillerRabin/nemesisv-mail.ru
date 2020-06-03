#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "DragonSlayer";
const char* password = "ifyouwanttohave";

ESP8266WebServer server(80);


Ticker lightTicker;
int gBrightness = 100;
bool gOn = false;
int gCounter = 0;
#define lamp 10

void tick()
{    
  if (!gOn) {
    digitalWrite(lamp, 0);
    return;
  }

  gCounter = (gCounter + 1) % 100;  
  if (gCounter >= gBrightness) {
    digitalWrite(lamp, 0);
    return;
  }
  digitalWrite(lamp, 1);    
}

int range(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}


void setParameters(String name, int value) {
  if (name == "brightness") {
    gCounter = 0;    
    gBrightness = range(value, 0, 100);
  }
  if (name == "on") {
    gOn = bool(value);
    gCounter = 0;    
  }
}

String formatResponse() {
  String onStr = gOn ? "1" : "0";

  return "{ \"on\":" + onStr + ",\n" + 
            "\"brightness\":" + String(gBrightness) + "\n" + 
          "}";
}

void handleRoot() {  
  for (uint8_t i = 0; i < server.args(); i++) {
    unsigned int val = server.arg(i).toInt();
    setParameters(server.argName(i), val); 
  }
  
  server.sendHeader("Access-Control-Allow-Origin",  "*");   
  String res = formatResponse();
  server.send(200, "application/json", res);  
}

void handleNotFound(){  
  String res = formatResponse();
  server.sendHeader("Access-Control-Allow-Origin",  "*");   
  server.send(404, "application/json", res);  
}

void setupWiFI() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.addService("http", "tcp", 80);
  if (!MDNS.begin("lamp")) {
    Serial.println("MDNS responder not started");
  } else {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void setup()
{  
  pinMode(lamp, OUTPUT);
  Serial.begin(115200);
  setupWiFI();
  lightTicker.attach_ms(200, tick);
}

void loop(){
  server.handleClient();
}