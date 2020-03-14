//#pragma message "Compiling " __FILE__ "..."
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Waddress"

#include <Arduino.h>
#include <SPI.h>

#if defined(USE_UCGLIB)
#include <Ucglib.h>
// #elif defined(SCREEN_VIRTUAL)
// #include <GlcdRemoteClient.h>
#else
#include <U8g2lib.h>
#endif

#include <JC_Button.h>

// #include <Adafruit_Sensor.h>
// #include <DHT.h>

#pragma GCC diagnostic pop

#include "mUI.h"

//#define PLATFORM_SAMD21MINI
//#define PLATFORM_M5STACK

#define ESPRESET_PIN 13
#define LED1 25 // TX
#define LED2 26 // RX

#define WIDTH 128
#define HEIGHT 64

#define DEBOUNCE_MS 10

#if defined(PLATFORM_M5STACK)
//#pragma message "Compiling " __FILE__ " for M5Stack"
#define TFT_DC 27
#define TFT_CS 14
#define TFT_RST 33
#define TFT_MOSI_MISO 27
#define TFT_CLK 14
#define TFT_BL 32

// #if defined(SCREEN_ILI9342)
Ucglib_ILI9341_18x240x320_HWSPI u8g2(/*cd=*/TFT_DC, /*cs=*/TFT_CS, /*reset=*/TFT_RST);
// #elif defined(SCREEN_VIRTUAL)
// GlcdRemoteClient u8g2(U8G2_R0, u8g2_Setup_ssd1309_128x64_noname2_f, COMM_SERIAL);
// #endif

#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 38
#define BUTTON_C_PIN 37

#define SerialUSB Serial

#elif defined(PLATFORM_SAMD21MINI)
//#pragma message "Compiling " __FILE__ " for SAMD21Mini"

#if defined(SCREEN_ST7735)
Ucglib_ST7735_18x128x160_HWSPI u8g2(/*cd=*/9, /*cs=*/10, /*reset=*/8);
#elif defined(SCREEN_SSD1306)
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(/*rotation=*/U8G2_R0, /*reset=*/U8X8_PIN_NONE);
#elif defined(SCREEN_SSD1309)
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(/*rotation=*/U8G2_R0, /*cs=*/10, /*dc=*/9, /*reset=*/8);
#else
#pragma GCC error "no display defined"
#endif

#define BUTTON_A_PIN 2
#define BUTTON_B_PIN 3
#define BUTTON_C_PIN 4
#else
#pragma GCC error "no platform defined"
#endif

Button BtnA = Button(BUTTON_A_PIN);
Button BtnB = Button(BUTTON_B_PIN);
Button BtnC = Button(BUTTON_C_PIN);

// #define DHTPIN 5
// DHT dht(DHTPIN, DHT11);

char inChar;
char serial_buf[50];
char serial_buf1[50];
char serial_buf2[50];
char ui_buf[50];
bool WiFi_on = false;
int time_temp;


uint8_t test_buttons()
{
  BtnA.read();
  BtnB.read();
  BtnC.read();
  return (BtnC.wasReleased() << 2) | (BtnB.wasReleased() << 1) | (BtnA.wasReleased() << 0);
}

void setup()
{

#if defined(PLATFORM_M5STACK)
  Serial.begin(9600);
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);

  // Setup backlight
  ledcSetup(7, 44100, 8);
  ledcAttachPin(TFT_BL, 7);
  ledcWrite(7, 80);
#endif

#if defined(PLATFORM_SAMD21MINI)
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  Serial1.setTimeout(100);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);
#endif

  digitalWrite(ESPRESET_PIN, HIGH);
  pinMode(ESPRESET_PIN, OUTPUT);

  // Initialize display
#if defined(USE_UCGLIB)
  u8g2.begin(UCG_FONT_MODE_SOLID);
  u8g2.setScale2x2();
#else
  u8g2.begin();
#endif

#if defined(PLATFORM_SAMD21MINI) && defined(USE_UCGLIB)
  u8g2.setRotate270();
#endif

  Serial1.write("D"); // Send ESP8266 into DeepSleep

  // dht.begin();

  mUI::begin(&u8g2);

#if defined(USE_U8G2)
  u8g2.setFont(u8g2_font_5x8_mr);
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();
#endif

  for (;;)
  {
    main_window.update();
  }
}

void loop()
{
}