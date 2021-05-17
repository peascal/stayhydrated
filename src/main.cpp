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

void setup()
{
  Serial.begin(115200);
  Serial.println();

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

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}