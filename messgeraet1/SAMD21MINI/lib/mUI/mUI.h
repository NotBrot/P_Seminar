/*
 *
 */

#ifndef _MUI_H_
#define _MUI_H_

#ifdef USE_U8G2
#include <U8g2lib.h>
#else
#include <Ucglib.h>
#endif

//#include <vector>

#define WIDTH 128
#define HEIGHT 64

#ifdef PLATFORM_M5STACK
#define BLACK 255, 255, 255
#define WHITE 0, 0, 0
#define GREEN 255, 0, 255
#define DARKGRAY 230, 230, 230
#else
#define BLACK 0, 0, 0
#define WHITE 255, 255, 255
#define GREEN 0, 255, 0
#define DARKGRAY 25, 25, 25
#endif

namespace mUI
{
enum WidgetType
{
  BUTTON,
  LABEL
};

struct Widget
{
  WidgetType type;
  uint8_t pos_x;
  uint8_t pos_y;
  uint8_t width;
  uint8_t height;
  const char *text;
  bool has_changed;
  uint8_t index;
};

struct Window
{
  //std::vector<Widget> widgets;
  const char *title;
  uint8_t num_of_widgets;
  Widget widgets[];
};

#ifdef USE_U8G2
static U8G2 *screen;
#else
static Ucglib *screen;
#endif

static uint8_t previous_button_index = 1;
static uint8_t current_button_index = 0;

// inline Ucglib *screen;
// inline uint8_t previous_button_index = 1;
// inline uint8_t current_button_index = 0;

#ifdef USE_U8G2
void begin(U8G2 *display);
#else
void begin(Ucglib *display); 
#endif

void begin();
void clearScreen();
void drawMenuFrame(const char *title);
void drawStatus(const char status);
void drawButton(const char index, const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text);
void drawText(const uint16_t pos_x, uint16_t pos_y, const char *text);
void drawListBox(const uint16_t pos_x, const uint16_t pos_y, const char *list_items[], const size_t size, const size_t current_index);
void drawPopup(const char* message, const uint16_t pos_x, const uint16_t pos_y);

void drawWindow(Window *window);
} // namespace mUI

#endif