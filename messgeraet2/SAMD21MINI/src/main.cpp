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
U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI(/*rotation=*/U8G2_R0, /*cs=*/10, /*dc=*/9, /*reset=*/8);
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

// void updateUI()
// {
//   BtnA.read();
//   BtnB.read();
//   BtnC.read();

//   //mUI::clearScreen();

//   mUI::drawMenuFrame("Test");
//   mUI::drawStatus('W');

//   if (BtnA.wasReleased())
//   {
//     if (mUI::current_button_index > 0)
//       mUI::current_button_index--;
//   }
//   else if (BtnB.wasReleased())
//   {
//     mUI::current_button_index++;
//   }
//   if (mUI::current_button_index != mUI::previous_button_index)
//   {
//     mUI::drawButton(0, 30, 20, 37, 15, "Test 1");
//     mUI::drawButton(1, 30, 40, 37, 15, "Test 2");
//   }
//   mUI::previous_button_index = mUI::current_button_index;
// }

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
  u8g2.clearBuffer();
#endif

  //u8g2.drawStr(0, 10, "Test");

  // mUI::Label label_data = {2, 22, 0, 0, "Test"};
  // mUI::Widget test_label = {mUI::WidgetType::LABEL, label_data};
  // test_label.data. = 2;
  // test_label.pos_y = 22;
  // test_label.type = mUI::WidgetType::LABEL;
  // // mUI::Label label;
  // // label.text = "Test";
  // // test_label.data.label = &label;
  // test_label.data.label.text = "Test";

  mUI::Label test;

  test.text = "Test";
  test.pos = {2, 22};

  //mUI::ListBox lb;
  mUI::ListItem lb_items[] = {mUI::ListItem("Test", [](mUI::Window &caller){mUI::MessageBox msgTest(caller, "Titel", "abc", mUI::INFO); msgTest.show();}), mUI::ListItem("Test2"), mUI::ListItem("Test3")};
  //lb.size = sizeof(lb_items) / sizeof(lb_items[0]);
  //lb.pos = {2, 12};
  mUI::ListBox lb({0, 12}, sizeof(lb_items) / sizeof(lb_items[0]), lb_items);

  mUI::Button btn0(mUI::vec2(2, 12), {26, 14}, 0, "0001", [](mUI::Window &caller){mUI::drawStatus('1');});
  mUI::Button btn1({2, 28}, {26, 14}, 1, "0002", [](mUI::Window &caller){mUI::drawStatus('2');});
  mUI::Button btn2({2, 44}, {26, 14}, 2, "0003", [](mUI::Window &caller){mUI::drawStatus('3');});
  mUI::Button btn3({30, 12}, {26, 14}, 3, "0004", [](mUI::Window &caller){mUI::drawStatus('4');});
  mUI::Button btn4({30, 28}, {26, 14}, 4, "0005", [](mUI::Window &caller){mUI::drawStatus('5');});

  //mUI::Widget *wdgts[] = {&btn0, &btn1, &btn2, &btn3, &btn4};
  mUI::Widget *wdgts[] = {&lb};

  //bool (*button_tests[])() = {BtnA.wasReleased, BtnB.wasReleased, BtnC.wasReleased};

  mUI::Window main_window("mUI-Test", test_buttons, sizeof(wdgts) / sizeof(wdgts[0]), wdgts);
  main_window.update();


  //mUI::MessageBox msgTest(main_window, "Titel", "Test", mUI::INFO);

  btn4.on_select = [](mUI::Window &caller){mUI::MessageBox msgTest(caller, "Titel", String(u8g2.getStrWidth("0001")).c_str(), mUI::INFO); msgTest.show();};
  // mUI::Window main_window = {"mUI-Test", 1, {test_label}};

  // main_window.title = "mUI-Test";
  // main_window.num_of_widgets = 1;
  // main_window.widgets = {test_label};
  // mUI::drawWindow(main_window);
  // mUI::drawText(0, 10, "Test");
#if defined(USE_U8G2)
  u8g2.sendBuffer();
#endif
  //mUI::drawButton(0, 30, 20, 37, 15, "Test 1");
  //mUI::drawText(2, 22, "Test2");
  for (;;)
  {
    main_window.update();
  }
}

void loop()
{
//   //updateUI();
//   BtnA.read();
//   BtnB.read();
//   BtnC.read();

//   if (BtnA.wasReleased())
//   {
//     if (mUI::current_button_index > 0)
//       mUI::current_button_index--;
//   }
//   else if (BtnB.wasReleased())
//   {
//     mUI::current_button_index++;
//   }
//   else if (BtnC.wasReleased())
//   {
//     switch (mUI::current_button_index)
//     {
//     case 0:
//       Serial1.write("T");
//       SerialUSB.println("Button 0");
//       break;

//     case 1:
//       mUI::clearScreen();
//       mUI::drawMenuFrame("DHT11-Test");
//       time_temp = millis();
//       mUI::previous_button_index = 255;
//       for (;;)
//       {
//         BtnA.read();
//         BtnB.read();
//         BtnC.read();

//         if (millis() - time_temp > 300)
//         {
//           mUI::clearScreen();
//           mUI::drawMenuFrame("DHT11-Test");
//           sprintf(ui_buf, "Temperatur: %.2f°C", dht.readTemperature());
//           mUI::drawText(3, 13, ui_buf);
//           sprintf(ui_buf, "Luftfeuchigkeit: %.2f%%", dht.readHumidity());
//           mUI::drawText(3, 23, ui_buf);
//           time_temp = millis();
//         }

//         if (BtnA.wasReleased() || BtnB.wasReleased() || BtnC.wasReleased())
//         {
//           break;
//         }
//       }
//       break;

//     case 2:
//       if (WiFi_on) // WiFi already on
//       {
//         mUI::drawPopup("WLAN bereits an!", 5, 20);
//         mUI::previous_button_index = 255;
//         delay(500);
//         break;
//       }
//       mUI::drawPopup("WLAN wird angeschaltet", 5, 20);
//       mUI::previous_button_index = 255;
//       SerialUSB.println("Waking up ESP8266...");
//       digitalWrite(ESPRESET_PIN, LOW);
//       delay(10);
//       digitalWrite(ESPRESET_PIN, HIGH);
//       delay(30);
//       while (Serial1.read() != 'O')
//         ; // Wait for ACK from ESP8266
//       WiFi_on = true;
//       break;

//     case 3:
//       mUI::drawPopup("WLAN wird ausgeschaltet", 5, 20);
//       mUI::previous_button_index = 255;
//       Serial1.write("D");
//       SerialUSB.println("Sending ESP8266 into DeepSleep...");
//       delay(100);
//       WiFi_on = false;
//       break;

//     case 4:
//       Serial1.write("I"); // Request Info
//       Serial1.flush();    // Wait until transfer complete
//       //delay(50);            // Give the ESP time to print AP info
//       serial_buf[0] = '\0'; // Clear string
//       //while (Serial1.available() <= 0); // Wait for communication start
//       for (;;)
//       {
//         while (Serial1.available() <= 0)
//           ; // Wait for incoming data
//         inChar = (char)Serial1.read();
//         if (inChar == '\n')
//           break;
//         strncat(serial_buf, &inChar, 1);
//       }

//       Serial1.write("O"); // Send ACK
//       Serial1.flush();
//       //delay(50);
//       serial_buf1[0] = '\0';
//       //while (Serial1.available() <= 0);
//       for (;;)
//       {
//         while (Serial1.available() <= 0)
//           ;
//         inChar = (char)Serial1.read();
//         if (inChar == '\n')
//           break;
//         strncat(serial_buf1, &inChar, 1);
//       }

//       sprintf(ui_buf, "AP: \"%s\"\nPass: \"%s\"", serial_buf, serial_buf1);

//       Serial1.write("O"); // Send ACK
//       Serial1.flush();

//       mUI::drawPopup(ui_buf, 5, 20);
//       delay(500);
//       break;

//     default:
//       break;
//     }
//   }

//   while (Serial1.available() > 0)
//   {
//     inChar = (char)Serial1.read();
//     switch (inChar)
//     {
//     case 'R':    // Request Data
//       delay(10); // Give ESP32 time to send next char
//       inChar = (char)Serial1.read();
//       switch (inChar)
//       {
//       case 'T': // Request Temperature
//         sprintf(serial_buf, "%.2f", dht.readTemperature());
//         Serial1.write(serial_buf);
//         break;

//       case 'H': // Request Humidity
//         sprintf(serial_buf, "%.2f", dht.readHumidity());
//         Serial1.write(serial_buf);
//         break;

//       default:
//         //mUI::drawStatus(inChar);
//         mUI::drawStatus('E');
//         break;
//       }
//       break;

//     default:
//       mUI::drawStatus(inChar);
//       break;
//     }
//   }

//   Serial1.flush();

//   const char *list_items[] = {"Test", "DHT11-Test", "WLAN an", "WLAN aus", "WLAN-Info", "Test 5", "Test 6"};
//   size_t list_itmes_size = sizeof(list_items) / sizeof(list_items[0]);

//   if (mUI::current_button_index != mUI::previous_button_index)
//   {
//     mUI::clearScreen();
//     mUI::drawMenuFrame("mUI-Test");

//     mUI::drawListBox(2, 12, list_items, list_itmes_size, mUI::current_button_index);
//   }
//   mUI::previous_button_index = mUI::current_button_index;

// #if defined(USE_U8G2)
//   u8g2.sendBuffer();
// #endif
}