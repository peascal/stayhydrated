#include "WiFi.h"
#include "WebServer.h"

#include "Settings.h" 

WebServer server(80);



String SendHTML()
{
  String html = "<!DOCTYPE html> <html>\n";
  html +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html +="<title>Stay hydrated</title>\n";
  html +="</head>\n";
  html +="<body>\n";
  html +="<h1>ESP32 Web Server</h1>\n";
  html +="<h3>Using Access Point(AP) Mode</h3>\n";
  html +="</body>\n";
  html +="</html>\n";
  return html;
}

void handle_OnConnect()
{
  Serial.println("server requested");
  server.send(200, "text/html", SendHTML()); 
}

void pumpOn()
{
  digitalWrite(PIN_PUMP, LOW);
}

void pumpOff()
{
  digitalWrite(PIN_PUMP, HIGH);
}

void ventilOpen(int id_int)
{
  int ventil_pin;

  ventil_pin = -1;

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
  }
  
  if (ventil_pin != -1) {
    digitalWrite(ventil_pin, LOW);
  }
}

void ventilClose(int id_int)
{
  int ventil_pin;

  ventil_pin = -1;

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
  }

  if (ventil_pin != -1) {
    digitalWrite(ventil_pin, HIGH);
  }
}

void hydrate(String id)
{
  int id_int;
  id_int = id.toInt();

  ventilOpen(id_int);
  pumpOn();

  delay(2 * 1000);

  pumpOff();
  ventilClose(id_int);
}

void handle_Hydrate()
{
  String message;
  String id;

  message = "plant id = ";
  id = server.arg("plant");

  message += id;

  hydrate(id);

  server.send(200, "text/html", message);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

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
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/hydrate", handle_Hydrate);

  server.begin();
  Serial.println("HTTP server started");
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
  }
  if (capaPin == -1)
  {
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
void loop() 
{
  server.handleClient();
 
  delay(1000);
}