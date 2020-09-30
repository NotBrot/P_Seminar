/*
 *
 */

#ifndef _MUI_H_
#define _MUI_H_

#include <U8g2lib.h>

//#include <vector>

#define mUI_VERSION_STRING "0.1a"

#define WIDTH 128
#define HEIGHT 64

namespace mUI
{

class Window;

// enum WidgetType
// {
//   LISTBOX,
//   BUTTON,
//   LABEL
// };

enum WidgetType
{
  WIDGET,
  SELECTABLEWIDGET,
  LISTITEM,
  MENUENTRY,
  SUBMENUENTRY
};

struct vec2
{
  uint8_t x, y;

  vec2(uint8_t x, uint8_t y)
      : x(x), y(y) {}
};

class Widget
{
public:
  vec2 pos = {0, 0}, size = {0, 0};
  bool redraw = true;
  WidgetType type;
  virtual void draw(Window &parent);
  virtual void update(Window &parent);

  Widget(WidgetType type)
      : type(type) {}
  Widget(WidgetType type, vec2 pos, vec2 size)
      : pos(pos), size(size), type(type) {}
};

class Window
{
  //private:
public:
  const char *title = "";
  uint8_t num_of_widgets = 0;
  uint8_t current_index = 0;
  uint8_t previous_index = 0;
  uint8_t (*test_buttons)();
  Widget **widgets = {};
  // Widget *selectable_widgets[];

  union button_states_t {
    struct bits
    {
      uint8_t A : 1, B : 1, C : 1;
    } bit;
    uint8_t value;
  } button_states;
  bool redraw_all = true;
  bool redraw_frame = true;

  //public:
  Window() {}
  Window(const char *title, uint8_t (*test_buttons)(), uint8_t num_of_widgets, Widget *widgets[]);
  void update(bool force = false);
};

class SelectableWidget : public Widget
{
public:
  uint8_t index = 0;
  bool selected = false;
  void (*on_select)(Window &) = nullptr;

  SelectableWidget()
      : Widget(WidgetType::SELECTABLEWIDGET) {}
  SelectableWidget(WidgetType type)
      : Widget(type) {}
  SelectableWidget(vec2 pos, vec2 size, uint8_t index, void (*on_select)(Window &))
      : Widget(WidgetType::SELECTABLEWIDGET, pos, size), index(index), on_select(on_select) {}
  SelectableWidget(WidgetType type, vec2 pos, vec2 size, uint8_t index, void (*on_select)(Window &))
      : Widget(type, pos, size), index(index), on_select(on_select) {}
};

class CapturingWidget : public SelectableWidget
{
public:
  uint8_t current_index = 0;
  uint8_t previous_index = 0;
  uint8_t number_of_items = 0;
  bool captured = false;
  Widget *items = {};
  
  CapturingWidget() {}
  CapturingWidget(vec2 pos, vec2 size, uint8_t number_of_items, Widget items[], uint8_t index)
      : SelectableWidget(pos, size, index, nullptr), number_of_items(number_of_items), items(items) {}
};

class ListItem : public SelectableWidget
{
public:
  const char *text;
  
  bool is_checkbox = false;
  bool checked = false;

  ListItem() {}

  ListItem(WidgetType type, const char *text)
      : SelectableWidget(type, vec2(0, 0), vec2(0, 0), 0, nullptr), text(text) {}
  ListItem(WidgetType type, const char *text, void (*on_select)(Window &))
      : SelectableWidget(type, vec2(0, 0), vec2(0, 0), 0, on_select), text(text) {}

  ListItem(const char *text)
      : SelectableWidget(WidgetType::LISTITEM, vec2(0, 0), vec2(0, 0), 0, nullptr), text(text) {}
  ListItem(const char *text, void (*on_select)(Window &))
      : SelectableWidget(WidgetType::LISTITEM, vec2(0, 0), vec2(0, 0), 0, on_select), text(text) {}
};

// class ListBox : public CapturingWidget
// {
// public:
//   ListBox() {}
//   ListBox(uint8_t size, uint8_t index,  ListItem items[])
//       : CapturingWidget(vec2(2, 12), vec2(0, 0), size, items, index) {}
//   ListBox(vec2 pos, uint8_t size, ListItem items[])
//       : CapturingWidget(pos, vec2(0, 0), size, items, index) {}

//   void draw() override;
//   //void update() override;
// };

class ListBox : public SelectableWidget
{
public:
  uint8_t list_index = 0;
  uint8_t previous_index = 0;
  uint8_t size = 0;
  ListItem *items = {};

  ListBox() {}
  ListBox(uint8_t size, ListItem items[])
      : SelectableWidget(vec2(2, 14), vec2(0, 0), 0, nullptr), size(size), items(items) {}
  ListBox(vec2 pos, uint8_t size, ListItem items[])
      : SelectableWidget(pos, vec2(0, 0), 0, nullptr), size(size), items(items) {}

  void draw(Window &parent) override;
};

class MenuEntry : public ListItem
{
  
  MenuEntry() {}

  MenuEntry(const char *text)
      : ListItem(WidgetType::MENUENTRY, text) {}
  MenuEntry(const char *text, void (*on_select)(Window &))
      : ListItem(WidgetType::MENUENTRY, text, on_select) {}
};

class SubMenuEntry : public MenuEntry
{

};

class MenuWindow : public Window
{
  MenuWindow(const char *title, uint8_t (*test_buttons)(), uint8_t num_of_entries, MenuEntry *menu_entries[]);
};

class Button : public SelectableWidget
{
public:
  const char *text = "";

  Button() {}
  Button(vec2 pos, vec2 size, uint8_t index, const char *text)
      : SelectableWidget(pos, size, index, nullptr), text(text) {}
  Button(vec2 pos, vec2 size, uint8_t index, const char *text, void (*on_select)(Window &))
      : SelectableWidget(pos, size, index, on_select), text(text) {}

  void draw(Window &parent) override;
  //void update() override;
};

class Label : public Widget
{
public:
  const char *text = "";
  const uint8_t *font = u8g2_font_5x8_mf;

  Label()
      : Widget(WidgetType::WIDGET) {}
  Label(vec2 pos, vec2 size, const char *text)
      : Widget(WidgetType::WIDGET, pos, size), text(text) {}

  void draw(Window &parent) override;
  //void update() override;
};

enum MessageBoxType
{
  INFO,
  WARNING,
  ERROR
};

class MessageBox
{
private:
  const char *title = "";
  const char *message = "";
  Window &parent;

public:
  MessageBox(Window &parent)
      : parent(parent) {}
  MessageBox(Window &parent, const char *title, const char *message, MessageBoxType message_box_type)
      : title(title), message(message), parent(parent) {}

  void show();
};

// struct ListBoxItem
// {
//   uint8_t index = 0;
//   const char *text = "";
//   void (*on_select)() = nullptr;
// };

// struct ListBox
// {
//   uint8_t x, y, w, h;
//   bool redraw = true;
//   uint8_t index = 0;
//   uint8_t list_index = 0;
//   uint8_t size = 0;
//   ListBoxItem items[] = {};
// };

// struct Button
// {
//   uint8_t x, y, w, h;
//   bool redraw = true;
//   uint8_t index = 0;
//   const char *text = "";
//   void (*on_select)() = nullptr;
// };

// struct Label
// {
//   uint8_t x, y, w, h;
//   bool redraw = true;
//   const char *text = "";
// };

// typedef union {
//   ListBox *list_box;
//   Button *button;
//   Label *label;
// } WidgetData;

// struct Widget
// {
//   WidgetType type;

//   // uint8_t pos_x;
//   // uint8_t pos_y;
//   // uint8_t width;
//   // uint8_t height;
//   // const char *text;
//   // bool draw_needed = true;
//   // uint8_t index;
//   WidgetData data;
// };

// struct Window
// {
//   //std::vector<Widget> widgets;
//   const char *title;
//   uint8_t num_of_widgets;
//   Widget widgets[20];
// };

typedef void (*OnSelectHandler)(Window &);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static U8G2 *screen;

static uint16_t status_symbols[3] = {0x0, 0x0, 0x0};
void setStatus(uint8_t index, uint16_t value);

#pragma GCC diagnostic pop

// static uint8_t previous_button_index = 1;
// static uint8_t current_button_index = 0;

// inline uint8_t previous_button_index = 1;
// inline uint8_t current_button_index = 0;

void begin(U8G2 *display);

void begin();
void clearScreen();
void drawMenuFrame(const char *title);
void drawStatus(uint16_t status[3]);
void drawButton(const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text, bool selected);
void drawText(const uint16_t pos_x, uint16_t pos_y, const char *text);
void drawListBox(const uint16_t pos_x, const uint16_t pos_y, const ListItem list_items[], const size_t size, const size_t current_index);
// void drawListBox(const uint16_t pos_x, const uint16_t pos_y, const Widget list_items[], const size_t size, const size_t current_index);
void drawPopup(const char *message, const uint16_t pos_x, const uint16_t pos_y);

void drawWindow(Window &window);
} // namespace mUI

#endif