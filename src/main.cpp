#include "WiFi.h"
#include "WebServer.h"

#include "Settings.h" 

WebServer server(80);

String SendHTML(){
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

void handle_OnConnect() {
  Serial.println("server requested");
  server.send(200, "text/html", SendHTML()); 
}

void hydrate (String id)
{
  int id_int;
  int ventil_pin;

  ventil_pin = NULL;

  id_int = id.toInt();

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

  if (ventil_pin == NULL) {
    return;
  }

  digitalWrite(ventil_pin, HIGH);
  digitalWrite(PIN_PUMP, HIGH);

  delay(2 * 1000);

  digitalWrite(PIN_PUMP, LOW);
  digitalWrite(ventil_pin, LOW);
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

void loop() {
  server.handleClient();
}