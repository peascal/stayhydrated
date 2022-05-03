#include "WiFi.h"
#include "WebServer.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "Settings.h" 

AsyncWebServer server(80);

void pumpOn()
{
  digitalWrite(2, HIGH);
  digitalWrite(PIN_PUMP, LOW);
}

void pumpOff()
{
  digitalWrite(2, LOW);
  digitalWrite(PIN_PUMP, HIGH);
}

int getWaterLevelInCm()
{
  const int SENSOR_MAX_RANGE = 300; // in cm
  unsigned long duration;
  unsigned int distance;
  int totalDistance = 0;
  int success = 0;

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(PIN_ULTRASCHALL_TRIGGER, LOW);
    delayMicroseconds(2);

    digitalWrite(PIN_ULTRASCHALL_TRIGGER, HIGH);
    delayMicroseconds(10);

    duration = pulseIn(PIN_ULTRASCHALL_ECHO, HIGH);
    distance = duration/58;
    if (distance < SENSOR_MAX_RANGE && distance > 2) 
    {
      totalDistance += distance;
      success++;
    }
  }
  if (success == 0)
  {
    return 0;
  }
  return totalDistance / success;
}


bool ventilOpen(int id_int)
{
  int ventil_pin = -1;

  switch (id_int)
  {
    case 1:
      ventil_pin = PIN_VENTIL_1;
      break;
    case 2:
      ventil_pin = PIN_VENTIL_2;
      break;
    case 3:
      ventil_pin = PIN_VENTIL_3;
      break;
    case 4:
      ventil_pin = PIN_VENTIL_4;
      break;
    default:
      return false;
  }
  
  digitalWrite(ventil_pin, LOW);
  return true;
}

bool ventilClose(int id_int)
{
  int ventil_pin = -1;

  switch (id_int)
  {
    case 1:
      ventil_pin = PIN_VENTIL_1;
      break;
    case 2:
      ventil_pin = PIN_VENTIL_2;
      break;
    case 3:
      ventil_pin = PIN_VENTIL_3;
      break;
    case 4:
      ventil_pin = PIN_VENTIL_4;
      break;
    default:
      return false;
  }

  digitalWrite(ventil_pin, HIGH);
  return true;
}

String hydrate(int id)
{ 
  if (getWaterLevelInCm() > MAX_WATERSURFACE_TO_ULTRASONIC_SENSOR_IN_CM) {
    return "low water level";
  }

  if(ventilOpen(id) == false) {
    return "ventil can not be opened";
  }

  pumpOn();

  delay(2 * 1000);

  pumpOff();
  
  if (ventilClose(id) == false) {
    return "ventil can not be closed";
  }

  return "success";
}


int getMoisture(int id_int)
{
  int water = 1330;
  int air = 3630;
  int interval = (air - water) / 10;
  int capaPin = -1; 
  switch (id_int)
  {
    case 1:
      capaPin = PIN_CAPA_1;
      break;
    case 2:
      capaPin = PIN_CAPA_2;
      break;
    case 3:
      capaPin = PIN_CAPA_3;
      break;
    case 4:
      capaPin = PIN_CAPA_4;
      break;
    default:
      return 0;
  }

  int moisture = (analogRead(capaPin) - water) / interval;
  if (moisture < 1)
  {
    moisture = 1;
  }
  if (moisture > 10)
  {
    moisture = 10;
  }
  return moisture;
}

void bigbrain()
{
  for (int i=1; i<=4; i++)
  {
    if(getMoisture(i) >= 6)
    {
      hydrate(i);
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(2, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_VENTIL_1, OUTPUT);
  pinMode(PIN_VENTIL_2, OUTPUT);
  pinMode(PIN_VENTIL_3, OUTPUT);
  pinMode(PIN_VENTIL_4, OUTPUT);
  pinMode(PIN_ULTRASCHALL_ECHO, INPUT);
  pinMode(PIN_ULTRASCHALL_TRIGGER, OUTPUT);
  

  pumpOff();
  ventilClose(1);
  ventilClose(2);
  ventilClose(3);
  ventilClose(4);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    // blink blue led when connecting
    digitalWrite(2, HIGH);
    delay(250);
    digitalWrite(2, LOW);
    delay(250);
  }

  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

  // Route to load css file
  server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/main.css", "text/css");
  });

  // Route to load js file
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/main.js", "application/javascript");
  });

  server.on("/hydrate", HTTP_GET, [](AsyncWebServerRequest *request){
    String id = request->arg("plant");
    String successMessage = hydrate(id.toInt()); 

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    if (successMessage == "success")
    {
      root["status"] = "success";
    }
    else 
    {
      root["status"] = "error";
      root["message"] = successMessage;
    }

    root.printTo(*response);
    request->send(response);
  });

  server.on("/waterlevel", HTTP_GET, [](AsyncWebServerRequest *request){
    int waterlevel = getWaterLevelInCm();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    if (waterlevel == 0)
    {
      root["status"] = "error";
    }
    else 
    {
      root["status"] = "success";
      root["waterlevel"] = waterlevel;
    }

    root.printTo(*response);
    request->send(response);
  });

  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    String id = request->arg("plant");
    int moisture = getMoisture(id.toInt());

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    if (moisture == 0)
    {
      root["status"] = "error";
    }
    else 
    {
      root["status"] = "success";
      root["moisture"] = moisture;
    }

    root.printTo(*response);
    request->send(response);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() 
{
  bigbrain();
}