#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <SDFS.h>

#include <DNSServer.h>

#include <U8g2lib.h>

#include "SerialTransfer.hpp"

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);

U8G2LOG u8g2log;

#define WIDTH 12
#define HEIGHT 7

uint8_t u8g2log_buf[WIDTH * HEIGHT];

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

ESP8266WebServer server(80);

const IPAddress ip(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

const char *ssid PROGMEM = "ESP-AP";
const char *password PROGMEM = "123456789";
const char *hostname PROGMEM = "esp";

const uint8_t DNS_PORT = 53;

DNSServer dns;

SerialTransfer::SerialHandler handler_list[] = {
    SerialTransfer::SerialHandler(SerialTransfer::SerialData('D', false), []() {
      //esp_deep_sleep_start();
      u8g2.setPowerSave(1);
      ESP.deepSleep(0);
    }),
    SerialTransfer::SerialHandler(SerialTransfer::SerialData('I', true), []() {
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
    })
};

void handleRoot()
{
  u8g2log.print(" / ");
  server.send_P(200, "text/html", INDEX);
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(50);

  u8g2.begin();
  u8g2.setFont(u8g2_font_5x7_mf);
  u8g2log.begin(u8g2, WIDTH, HEIGHT, u8g2log_buf);
  u8g2log.setLineHeightOffset(0);
  u8g2log.setRedrawMode(1);

  //WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  dns.start(DNS_PORT, "*", ip);

  if (!MDNS.begin(hostname))
    ESP.restart();

  delay(10);

  u8g2log.printf("AP:\n\"%s\"\npass:\n\"%s\"\nIP addr:\n%s\n", ssid, password, WiFi.softAPIP().toString().c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/temp", HTTP_GET, []() {
    u8g2log.print(" /t ");
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
    u8g2log.print(" /h ");
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

  Serial.write("O");
}

void loop()
{
  dns.processNextRequest();
  server.handleClient();
  SerialTransfer::handle(Serial, handler_list, sizeof(handler_list) / sizeof(handler_list[0]), [](char cmd) {
    u8g2log.write(cmd);
  });

  //   while (Serial.available() > 0)
  //   {
  //     inChar = Serial.read();
  //     u8g2log.write(inChar);

  //     if (inChar <= 0)
  //       continue;

  //     if (inChar == 'D')
  //     {
  //       //esp_deep_sleep_start();
  //       u8g2.setPowerSave(1);
  //       ESP.deepSleep(0);
  //     }
  //     else if (inChar == 'I')
  //     {
  //       Serial.write(ssid);
  //       Serial.write('\n'); // Send NL as indicator for end of string
  //       Serial.flush();
  //       while (Serial.read() != 'O')
  //         ;
  //       Serial.write(password);
  //       Serial.write('\n');
  //       Serial.flush();
  //       while (Serial.read() != 'O')
  //         ;
  //       // Serial2.write(WiFi.softAPIP().toString().c_str());
  //       // while (Serial2.read() != 'O');
  //     }
  //   }

  // #if defined(PLATFORM_M5STACK)
  //   if (M5.BtnA.wasReleased())
  //   {
  //     Serial2.write("T");
  //     M5.Lcd.println("Test");
  //     Serial2.flush();
  //   }
  // #endif
}