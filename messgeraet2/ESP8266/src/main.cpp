#include <Arduino.h>

#if defined(PLATFORM_M5STACK)
#include <M5Stack.h>
#endif

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include <DNSServer.h>

#define RESET_VECTOR() ((void(const *)(void))0)()

const char *INDEX PROGMEM = R"""(<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
  </head>
  <body id="body">
    <h1>Web-Server-Test</h1>
    <h2>DHT11</h2>
    <span id="error-message" style="font-size: 10px;">
      <p>Temperatur: <span id="temperature"></span>&deg;C</p>
      <p>Luftfeuchtigkeit: <span id="humidity"></span>%</p>
      <button onclick="update()">Update</button>
    </span>
  </body>
  <script>
    function updateTemp() {
      const request = new XMLHttpRequest();
      request.open("GET", "/temp");
      request.send();
      console.log("Test");
      request.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("temperature").innerHTML = this.responseText;
        } else if (this.readyState == 4 && this.status != 200) {
          document.getElementById("error-message").innerHTML =
            "<h1>Error getting values</h1>";
        }
      };
    }
    function updateHum() {
      const request = new XMLHttpRequest();
      request.open("GET", "/hum");
      request.send();
      console.log("Test");
      request.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("humidity").innerHTML = this.responseText;
        } else if (this.readyState == 4 && this.status != 200) {
          document.getElementById("error-message").innerHTML =
            "<h1>Error getting values</h1>";
        }
      };
    }
    function update() {
      updateTemp();
      updateHum();
    }
    setInterval(update, 2000);
    update();
  </script>
</html>
)""";

char inChar;
char serial_buf[50];

#if defined(ESP8266)
ESP8266WebServer server(80);
#elif defined(ESP32)
WebServer server(80);
#endif

const IPAddress ip(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

const char *ssid PROGMEM = "ESP8266-AP";
const char *password PROGMEM = "123456789";
const char *hostname PROGMEM = "esp";

const uint8_t DNS_PORT = 53;

DNSServer dns;

void handleRoot()
{
#if defined(PLATFORM_M5STACK)
  M5.Lcd.print(" / ");
#endif
  server.send_P(200, "text/html", INDEX);
}

void setup()
{
#if defined(PLATFORM_M5STACK)
  M5.begin();
  Serial2.begin(9600);
  Serial2.setTimeout(50);
  Serial2.flush();

  M5.Lcd.clearDisplay(0x000000);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(F("WebServer-Test"));

  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("\nAP: \"%s\", Pass: \"%s\"\n", ssid, password);
#endif

  //WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  dns.start(DNS_PORT, "*", ip);

  if (!MDNS.begin(hostname))
    ESP.restart();

  delay(10);
#if defined(PLATFORM_M5STACK)
  M5.Lcd.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());
#endif

  server.on("/", HTTP_GET, handleRoot);
  server.on("/temp", HTTP_GET, []() {
#if defined(PLATFORM_M5STACK)
    M5.Lcd.print(" /t ");
    Serial2.write("RT");
#endif
    delay(100);           // Give the SAMD21MINI time to print info
    serial_buf[0] = '\0'; // Clear string
    while (Serial.available() > 0)
    {
      inChar = (char)Serial.read();
      strncat(serial_buf, &inChar, 1);
    }
    if (serial_buf[0] == '\0') //Check if string empty
      strcpy(serial_buf, "--CONNECTION ERROR--");
    server.send_P(200, "text/plain", serial_buf);
  });
  server.on("/hum", HTTP_GET, []() {
#if defined(PLATFORM_M5STACK)
    M5.Lcd.print(" /h ");
    Serial2.write("RH");
#endif
    delay(100);           // Give the SAMD21MINI time to print info
    serial_buf[0] = '\0'; // Clear string
    while (Serial.available() > 0)
    {
      inChar = (char)Serial.read();
      strncat(serial_buf, &inChar, 1);
    }
    if (serial_buf[0] == '\0') //Check if string empty
      strcpy(serial_buf, "--CONNECTION ERROR--");
    server.send_P(200, "text/plain", serial_buf);
  });
  server.begin();

  MDNS.addService("http", "tcp", 80);

#if defined(PLATFORM_M5STACK)
  Serial2.write("O");
  Serial2.flush();
#endif
}

void loop()
{
  dns.processNextRequest();
  server.handleClient();

#if defined(PLATFORM_M5STACK)
  M5.update();

  while (Serial2.available() > 0)
#else
  while (Serial.available() > 0)
#endif
  {
    inChar = Serial.read();
    if (inChar <= 0)
      continue;

#if defined(PLATFORM_M5STACK)
    M5.Lcd.print(inChar);
    if (inChar == '\r')
      M5.Lcd.print('\n');
#endif

    if (inChar == 'D')
      //esp_deep_sleep_start();
      ESP.deepSleep(0);
    else if (inChar == 'I')
    {
      Serial.write(ssid);
      Serial.write('\n'); // Send NL as indicator for end of string
      Serial.flush();
      while (Serial.read() != 'O')
        ;
      Serial.write(password);
      Serial.write('\n');
      Serial.flush();
      while (Serial.read() != 'O')
        ;
      // Serial2.write(WiFi.softAPIP().toString().c_str());
      // while (Serial2.read() != 'O');
    }
  }

#if defined(PLATFORM_M5STACK)
  if (M5.BtnA.wasReleased())
  {
    Serial2.write("T");
    M5.Lcd.println("Test");
    Serial2.flush();
  }
#endif
}