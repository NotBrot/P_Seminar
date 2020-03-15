#pragma region includes
#include <Arduino.h>
#include <SPI.h>

#if defined(USE_UCGLIB)
#include <Ucglib.h>
#else
#include <U8g2lib.h>
#endif

#include <JC_Button.h>


#include "mUI.h"
#pragma endregion includes


#pragma region defines_declarations

#define VERSION_STRING "v0.1"
#define VERSION 1

#define ESPRESET_PIN 13
#define LED1 25 // TX
#define LED2 26 // RX

#define WIDTH 128
#define HEIGHT 64

#define DEBOUNCE_MS 10

#if defined(PLATFORM_M5STACK)
#define TFT_DC 27
#define TFT_CS 14
#define TFT_RST 33
#define TFT_MOSI_MISO 27
#define TFT_CLK 14
#define TFT_BL 32

Ucglib_ILI9341_18x240x320_HWSPI u8g2(/*cd=*/TFT_DC, /*cs=*/TFT_CS, /*reset=*/TFT_RST);

#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 38
#define BUTTON_C_PIN 37

#define SerialUSB Serial

#elif defined(PLATFORM_SAMD21MINI)

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

#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;

bool close_flag = false;

char inChar;
char serial_buf[50];
char serial_buf1[50];
char serial_buf2[50];
char ui_buf[50];
bool WiFi_on = false;
int time_temp;

#pragma endregion defines_declarations

uint8_t test_buttons()
{
  BtnA.read();
  BtnB.read();
  BtnC.read();
  return (BtnC.wasReleased() << 2) | (BtnB.wasReleased() << 1) | (BtnA.wasReleased() << 0);
}

void setup()
{
#pragma region init_code
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

  Wire.begin();

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

  mUI::begin(&u8g2);

#if defined(USE_U8G2)
  u8g2.setFont(u8g2_font_5x8_mf);
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();
#endif
#pragma endregion init
  // MAIN MENU

#pragma region user_interface_declaration
  mUI::ListItem main_menu_listitems[5];
  main_menu_listitems[0] = mUI::ListItem("Messwerte", [](mUI::Window &caller) {
    // MEASURING WINDOW

    mUI::ListItem measure_window_listitems[3];
    measure_window_listitems[0] = mUI::ListItem("Temperatur", [](mUI::Window &caller) {
      if (!bmp.begin(0x76))
      {
        mUI::MessageBox err = mUI::MessageBox(caller, "Fehler", "BMP280 nicht gefunden!", mUI::MessageBoxType::INFO);
        err.show();
      }
      else
      {
        mUI::Label temp_lbl({4, 22}, {0, 0}, "");
        temp_lbl.font = u8g2_font_helvR18_tf;
        mUI::Widget *temp_window_widgets[] = {&temp_lbl};
        mUI::Window temp_window("Temperatur", test_buttons, sizeof(temp_window_widgets) / sizeof(temp_window_widgets[0]), temp_window_widgets);

        while (!test_buttons())
        {
          u8g2.clearBuffer();
          sprintf(ui_buf, "%.2f °C", bmp.readTemperature());

          char *sp = strchr(ui_buf, '.');
          *sp = ',';

          temp_lbl.text = ui_buf;
          temp_window.update(true);
          u8g2.sendBuffer();
        }
      }
    });
    measure_window_listitems[1] = mUI::ListItem("Luftdruck", [](mUI::Window &caller) {
      if (!bmp.begin(0x76))
      {
        mUI::MessageBox err = mUI::MessageBox(caller, "Fehler", "BMP280 nicht gefunden!", mUI::MessageBoxType::INFO);
        err.show();
      }
      else
      {
        mUI::Label pressure_lbl({4, 25}, {0, 0}, "");
        pressure_lbl.font = u8g2_font_helvR14_tf;
        mUI::Widget *pressure_window_widgets[] = {&pressure_lbl};
        mUI::Window pressure_window("Luftdruck", test_buttons, sizeof(pressure_window_widgets) / sizeof(pressure_window_widgets[0]), pressure_window_widgets);

        while (!test_buttons())
        {
          u8g2.clearBuffer();
          sprintf(ui_buf, "%.2f hPa", bmp.readPressure() / 100);

          char *sp = strchr(ui_buf, '.');
          *sp = ',';

          pressure_lbl.text = ui_buf;
          pressure_window.update(true);
          u8g2.sendBuffer();
        }
      }
    });
    measure_window_listitems[2] = mUI::ListItem("Zurück", [](mUI::Window &caller) {
      close_flag = true;
    });

    mUI::ListBox measure_window_listbox({0, 14}, sizeof(measure_window_listitems) / sizeof(measure_window_listitems[0]), measure_window_listitems);
    mUI::Widget *measure_window_widgets[] = {&measure_window_listbox};
    mUI::Window measure_window("Messwerte", test_buttons, sizeof(measure_window_widgets) / sizeof(measure_window_widgets[0]), measure_window_widgets);

    while (!close_flag)
    {
      u8g2.clearBuffer();
      measure_window.update(true);
      if (WiFi_on)
      {
        u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
        mUI::drawStatus(0xf8);
        u8g2.setFont(u8g2_font_5x8_mf);
      }
      u8g2.sendBuffer();
    }
    close_flag = false;
  });
  main_menu_listitems[1] = mUI::ListItem("WLAN an", [](mUI::Window &caller) {
    mUI::ListBox *l = (mUI::ListBox *)caller.widgets[0];
    l->items[1].checked = !l->items[1].checked;

    if (l->items[1].checked)
    {
      caller.update(true);
      mUI::drawPopup("WLAN wird angeschaltet", 5, 20);
      u8g2.sendBuffer();
      digitalWrite(ESPRESET_PIN, LOW);
      delay(10);
      digitalWrite(ESPRESET_PIN, HIGH);
      delay(30);
      while (Serial1.read() != 'O')
        ; // Wait for ACK from ESP8266
      WiFi_on = true;
      delay(700);
    }
    else
    {
      mUI::drawPopup("WLAN wird ausgeschaltet", 5, 20);
      Serial1.write("D");
      delay(100);
      WiFi_on = false;
    }
  });
  main_menu_listitems[1].is_checkbox = true;
  main_menu_listitems[2] = mUI::ListItem("WLAN-Info", [](mUI::Window &caller) {
    if (WiFi_on)
    {
      Serial1.write("I"); // Request Info
      Serial1.flush();    // Wait until transfer complete
      serial_buf[0] = '\0'; // Clear string
      for (;;)
      {
        while (Serial1.available() <= 0)
          ; // Wait for incoming data
        inChar = (char)Serial1.read();
        if (inChar == '\n')
          break;
        strncat(serial_buf, &inChar, 1);
      }

      Serial1.write("O"); // Send ACK
      Serial1.flush();
      serial_buf1[0] = '\0';
      for (;;)
      {
        while (Serial1.available() <= 0)
          ;
        inChar = (char)Serial1.read();
        if (inChar == '\n')
          break;
        strncat(serial_buf1, &inChar, 1);
      }

      sprintf(ui_buf, "AP: \"%s\"\nPass: \"%s\"", serial_buf, serial_buf1);

      Serial1.write("O"); // Send ACK
      Serial1.flush();

      mUI::MessageBox info = mUI::MessageBox(caller, "WLAN", ui_buf, mUI::MessageBoxType::INFO);
      info.show();
    }
    else
    {
      mUI::MessageBox err = mUI::MessageBox(caller, "WLAN", "WLAN ist deaktiviert!", mUI::MessageBoxType::INFO);
      err.show();
    }
  });
  main_menu_listitems[3] = mUI::ListItem("Debug", [](mUI::Window &caller) {
    // DEBUG WINDOW

    mUI::ListItem debug_window_listitems[4];
    debug_window_listitems[0] = mUI::ListItem("Sende \"T\" an ESP", [](mUI::Window &caller) {
      Serial1.print("T");
    });
    debug_window_listitems[1] = mUI::ListItem("I²C-Scanner", [](mUI::Window &caller) {
      byte error, address;
      int nDevices;

      u8g2.clearBuffer();
      u8g2.setCursor(0, 0);

      nDevices = 0;
      for (address = 1; address < 127; address++)
      {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
          if (address < 16)
            u8g2.print("0");
          u8g2.print(address, HEX);
          u8g2.print(" ");

          nDevices++;
        }
        else if (error == 4)
        {
          u8g2.print("?");
          if (address < 16)
            u8g2.print("0");
          u8g2.print(address, HEX);
          u8g2.print("? ");
        }
        u8g2.sendBuffer();
      }
      if (nDevices == 0)
        u8g2.println("Kein I²C-Slave gefunden");

      u8g2.sendBuffer();
      while (!test_buttons())
        ;
    });
    debug_window_listitems[2] = mUI::ListItem("Test", [](mUI::Window &caller) {
      mUI::Button but = mUI::Button({2, 14}, {(uint8_t)(u8g2.getStrWidth("ABC") + 6), (uint8_t)(u8g2.getMaxCharHeight() + 6)}, 0, "ABC");
      mUI::Button but2 = mUI::Button({30, 40}, {(uint8_t)(u8g2.getStrWidth("Test") + 6), (uint8_t)(u8g2.getMaxCharHeight() + 6)}, 1, "Test");
      mUI::Widget *test_window_widgets[] = {&but, &but2};
      mUI::Window test_window("Test", test_buttons, sizeof(test_window_widgets) / sizeof(test_window_widgets[0]), test_window_widgets);

      while (!close_flag)
      {
        u8g2.clearBuffer();
        but.pos.x = (uint8_t)(sin((double)millis() / 500) * (WIDTH - but.size.x) / 2 + (WIDTH - but.size.x) / 2);
        but2.pos.x = (uint8_t)(tan((double)millis() / 500) * (WIDTH - but.size.x) / 2 + (WIDTH - but.size.x) / 2);
        test_window.update(true);
        if (WiFi_on)
        {
          u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
          mUI::drawStatus(0xf8);
          u8g2.setFont(u8g2_font_5x8_mf);
        }
        u8g2.sendBuffer();
      }
      close_flag = false;
    });
    debug_window_listitems[3] = mUI::ListItem("Zurück", [](mUI::Window &caller) {
      close_flag = true;
    });

    mUI::ListBox debug_window_listbox({0, 14}, sizeof(debug_window_listitems) / sizeof(debug_window_listitems[0]), debug_window_listitems);
    mUI::Widget *debug_window_widgets[] = {&debug_window_listbox};
    mUI::Window debug_window("Debug", test_buttons, sizeof(debug_window_widgets) / sizeof(debug_window_widgets[0]), debug_window_widgets);

    while (!close_flag)
    {
      u8g2.clearBuffer();
      debug_window.update(true);
      if (WiFi_on)
      {
        u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
        mUI::drawStatus(0xf8);
        u8g2.setFont(u8g2_font_5x8_mf);
      }
      u8g2.sendBuffer();
    }
    close_flag = false;
  });
  main_menu_listitems[4] = mUI::ListItem("Info", [](mUI::Window &caller) {
    mUI::Label version_lbl({1, 12}, {0, 0}, "Version: " VERSION_STRING);
    mUI::Label build_lbl0({1, 22}, {0, 0}, "Build Time: ");
    mUI::Label build_lbl1({1, 32}, {0, 0}, __TIMESTAMP__);
    mUI::Widget *info_window_widgets[] = {&version_lbl, &build_lbl0, &build_lbl1};
    mUI::Window info_window("Info", test_buttons, sizeof(info_window_widgets) / sizeof(info_window_widgets[0]), info_window_widgets);

    u8g2.clearBuffer();
    info_window.update(true);
    u8g2.sendBuffer();

    while (!test_buttons())
      ;
  });

  mUI::ListBox main_menu_listbox({0, 14}, sizeof(main_menu_listitems) / sizeof(main_menu_listitems[0]), main_menu_listitems);
  mUI::Widget *main_menu_widgets[] = {&main_menu_listbox};
  mUI::Window main_menu("Hauptmenü", test_buttons, sizeof(main_menu_widgets) / sizeof(main_menu_widgets[0]), main_menu_widgets);
#pragma endregion user_interface_declaration

#pragma region main_loop
  for (;;)
  {
#if defined(USE_U8G2)
    u8g2.clearBuffer();
    main_menu.update(true);
    if (WiFi_on)
    {
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      mUI::drawStatus(0xf8);
      u8g2.setFont(u8g2_font_5x8_mf);
    }
    u8g2.sendBuffer();
#else
    main_window.update();
#endif
    while (Serial1.available())
    {
      inChar = (char)Serial1.read();
      switch (inChar)
      {
      case 'R':    // Request Data
        delay(10); // Give ESP8266 time to send next char
        inChar = (char)Serial1.read();
        switch (inChar)
        {
          // case 'T': // Request Temperature
          //   sprintf(serial_buf, "%.2f", dht.readTemperature());
          //   Serial1.write(serial_buf);
          //   break;

          // case 'H': // Request Humidity
          //   sprintf(serial_buf, "%.2f", dht.readHumidity());
          //   Serial1.write(serial_buf);
          //   break;

        default:
          mUI::drawStatus('E');
          break;
        }
        break;

      default:
        mUI::drawStatus(inChar);
        break;
      }
      u8g2.sendBuffer();
      delay(10);
    }
  }
#pragma endregion main_loop
}

void loop()
{
}