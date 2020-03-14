/*
 *
 */

#include "mUI.h"

void mUI::Widget::draw(Window &parent)
{
  drawText(2, 22, "?!?");
  return;
}

void mUI::Widget::update(mUI::Window &parent)
{
  if (redraw)
  {
    draw(parent);
    redraw = false;
  }
  return;
}

void mUI::Label::draw(Window &parent)
{
  screen->setFont(font);
  drawText(pos.x, pos.y, text);
  screen->setFont(u8g2_font_5x8_mf);
}

void mUI::ListBox::draw(Window &parent)
{
  if (parent.button_states.bit.A && list_index > 0)
  {
    list_index--;
    redraw = true;
  }
  if (parent.button_states.bit.B && list_index < size - 1)
  {
    list_index++;
    redraw = true;
  }
  if (parent.button_states.bit.C && items[list_index].on_select)
  {
    items[list_index].on_select(parent);
    redraw = true;
  }
  drawListBox(pos.x, pos.y, items, size, list_index);
}

// void mUI::ListBox::draw()
// {
//   drawListBox(pos.x, pos.y, items, number_of_items, current_index);
// }

void mUI::Button::draw(Window &parent)
{
  drawButton(pos.x, pos.y, size.x, size.y, text, selected);
}

mUI::Window::Window(const char *title, uint8_t (*test_buttons)(), uint8_t num_of_widgets, Widget **widget_list)
    : title(title), num_of_widgets(num_of_widgets), test_buttons(test_buttons), widgets(widget_list)
{
}

void mUI::Window::update(bool force /*=false*/)
{
  SelectableWidget *s;

  button_states.value = test_buttons();

  if (button_states.bit.A && current_index > 0)
    current_index--;
  if (button_states.bit.B && current_index < num_of_widgets - 1)
    current_index++;

  if (button_states.bit.C)
  {
    if (widgets[current_index]->type == WidgetType::SELECTABLEWIDGET)
    {
      s = (SelectableWidget *)(widgets[current_index]);
      if (s->on_select)
        s->on_select(*this);
    }
  }

  if (redraw_all)
  {
    for (int i = 0; i < num_of_widgets; i++)
    {
      widgets[i]->redraw = true;
      if (widgets[current_index]->type == WidgetType::SELECTABLEWIDGET)
      {
        s = (SelectableWidget *)(widgets[i]);
        s->selected = s->index == current_index;
      }
    }
    redraw_all = false;
  }

  else if (previous_index != current_index)
  {
    widgets[previous_index]->redraw = true;
    if (widgets[current_index]->type == WidgetType::SELECTABLEWIDGET)
    {
      s = (SelectableWidget *)(widgets[previous_index]);
      s->selected = s->index == current_index;
    }

    widgets[current_index]->redraw = true;
    if (widgets[current_index]->type == WidgetType::SELECTABLEWIDGET)
    {
      s = (SelectableWidget *)(widgets[current_index]);
      s->selected = s->index == current_index;
    }

    // for (int i = 0; i < num_of_widgets; i++)
    // {
    //   widgets[i]->redraw = true;
    //   s = dynamic_cast<SelectableWidget *>(widgets[i]);
    //   if (s)
    //     s->selected = s->index == current_index;
    // }
  }

  previous_index = current_index;

  if (redraw_frame || force)
  {
    drawMenuFrame(title);
    redraw_frame = false;
  }
  for (int i = 0; i < num_of_widgets; i++)
  {
    if (!force)
      widgets[i]->update(*this);
    else
      widgets[i]->draw(*this);
  }
}

void mUI::MessageBox::show()
{
  screen->clearBuffer();
  parent.update(true);
  drawPopup(message, 5, 20);
  screen->sendBuffer();
  delay(500);

#if defined(USE_UCGLIB)
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(5, 20, 120, 28);
  screen->setColor(0, GREEN); // Main Color: Green
#else
  screen->setDrawColor(0);
  screen->drawBox(5, 20, 120, 28);
  screen->setDrawColor(1);
#endif

  parent.update(true);
}

#if defined(USE_U8G2) || defined(SCREEN_VIRTUAL)
#if defined(USE_U8G2)
void mUI::begin(U8G2 *display)
// #else
// void mUI::begin(GlcdRemoteClient *display)
#endif
{
  screen = display;
  screen->clearDisplay();
  screen->setFont(u8g2_font_5x8_mf);
  screen->setFontPosTop(); // Set reference position for chars to top-left
}

void mUI::clearScreen()
{
  screen->clearBuffer();
  screen->clearDisplay();
}

void mUI::drawMenuFrame(const char *title)
{
  screen->drawFrame(0, 12, WIDTH, HEIGHT - 12);

  screen->setFont(u8g2_font_6x10_mf); // Bigger Font
  screen->drawUTF8(0, 1, title);
  screen->setFont(u8g2_font_5x8_mf);

  // screen->drawLine(0, 20, WIDTH - 1, 20);
}

void mUI::drawStatus(const char status)
{
  screen->drawGlyph(WIDTH - 9, 1, status);
}

void mUI::drawButton(const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text, bool selected)
{
  screen->setDrawColor(1);
  screen->drawFrame(pos_x, pos_y, w, h);

  if (selected)
    screen->drawFrame(pos_x + 2, pos_y + 2, w - 4, h - 4);

  screen->drawUTF8(pos_x + 3, pos_y + 3, text);
}

void mUI::drawText(const uint16_t pos_x, const uint16_t pos_y, const char *text)
{
  // screen->setCursor(pos_x + 3, pos_y + 3);
  // screen->print(text);
  screen->drawUTF8(pos_x + 3, pos_y + 3, text);
}

void mUI::drawListBox(const uint16_t pos_x, const uint16_t pos_y, const ListItem list_items[], const size_t size, const size_t current_index)
{
  for (size_t i = 0; i < size; ++i)
  {
    screen->setDrawColor(1);
    if (i > 4) // ListBox can only hold 5 items
    {
      break;
    }
    if (i == current_index)
    {
      screen->drawBox(pos_x + 1, pos_y + i * 10 - 1, WIDTH - 1, 10);
      screen->setDrawColor(0);
    }
    screen->drawUTF8(pos_x + 2, pos_y + i * 10, list_items[i].text);

    if (list_items[i].is_checkbox)
    {
      screen->setFont(u8g2_font_open_iconic_all_1x_t);
      screen->drawGlyph(WIDTH - 10, pos_y + i * 10, list_items[i].checked ? 0x73 : 0x11b);
      screen->setFont(u8g2_font_5x8_mf);
    }
    
    screen->setDrawColor(1);
    screen->drawLine(0, pos_y + i * 10 + 9, WIDTH - 1, pos_y + i * 10 + 9);
  }
}

// void mUI::drawListBox(const uint16_t pos_x, const uint16_t pos_y, const Widget list_items[], const size_t size, const size_t current_index)
// {
//   for (size_t i = 0; i < size; ++i)
//   {
//     if (i > 4) // ListBox can only hold 5 items
//     {
//       break;
//     }
//     if (i == current_index)
//     {
//       screen->setCursor(pos_x, pos_y + i * 10);
//       screen->print("-");
//     }
//     screen->setCursor(pos_x + 5, pos_y + i * 10);
//     screen->print(dynamic_cast<ListItem *>(list_items[i]).text);
//     screen->drawLine(0, pos_y + i * 10 + 9, WIDTH - 1, pos_y + i * 10 + 9);
//   }
// }

void mUI::drawPopup(const char *message, const uint16_t pos_x, const uint16_t pos_y)
{
  char *line;
  char msg_temp[50];
  strcpy(msg_temp, message);
  uint8_t lineno = 0;

  // Clear area
  screen->setDrawColor(0);
  screen->drawBox(pos_x, pos_y, 120, 28);

  screen->setDrawColor(1);
  screen->drawFrame(pos_x, pos_y, 120, 28);

  line = strtok(msg_temp, "\n");
  while (line != nullptr)
  {
    screen->drawUTF8(pos_x + 5, pos_y + 5 + lineno * 10, line);
    line = strtok(nullptr, "\n");
    lineno++;
  }
}

// void mUI::drawWindow(Window *window)
// {
//   drawMenuFrame(window->title);
//   for (uint8_t i = 0; i < window->num_of_widgets; i++)
//   {
//     switch (window->widgets[i].type)
//     {
//     case WidgetType::BUTTON:
//       if (window->widgets[i].draw_needed)
//       {
//         drawButton(window->widgets[i].index, window->widgets[i].pos_x, window->widgets[i].pos_y, window->widgets[i].width, window->widgets[i].height, window->widgets[i].text);
//       }
//       break;

//     case WidgetType::LABEL:
//       break;

//     default:
//       break;
//     }
//   }
// }

#else
void mUI::begin(Ucglib *display)
{
  screen = display;
  screen->clearScreen();
  screen->setFontPosTop(); // Set reference position for chars to top-left

  // Fill unused area dark gray
  screen->setColor(0, DARKGRAY); // Main Color: Dark gray
  screen->drawBox(0, 0, screen->getWidth(), screen->getHeight());

  // Fill screen black
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(0, 0, WIDTH, HEIGHT);

  screen->setColor(0, GREEN); // Main Color: Green
  screen->setColor(1, BLACK); // Background: Black
}

void mUI::clearScreen()
{
  // Fill screen black
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(0, 0, WIDTH, HEIGHT);

  screen->setColor(0, GREEN); // Main Color: Green
  screen->setColor(1, BLACK); // Background: Black
}

void mUI::drawMenuFrame(const char *title)
{
  screen->setFont(ucg_font_5x8_tr);

  screen->drawFrame(0, 10, WIDTH, HEIGHT - 10);

  screen->setPrintPos(0, 0);
  screen->print(title);

  // screen->drawLine(0, 20, WIDTH - 1, 20);
}

void mUI::drawStatus(const char status)
{
  screen->setPrintPos(WIDTH - 6, 1);
  screen->print(status);
}

void mUI::drawButton(const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text, bool selected)
{
  // Clear old button
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(pos_x, pos_y, w, h);

  screen->setColor(0, GREEN); // Main Color: Green
  screen->drawFrame(pos_x, pos_y, w, h);

  if (selected)
    screen->drawFrame(pos_x + 2, pos_y + 2, w - 4, h - 4);

  screen->setPrintPos(pos_x + 3, pos_y + 3);
  screen->print(text);
}

void mUI::drawText(const uint16_t pos_x, const uint16_t pos_y, const char *text)
{
  screen->setPrintPos(pos_x + 3, pos_y + 3);
  screen->print(text);
}

void mUI::drawListBox(const uint16_t pos_x, const uint16_t pos_y, const ListItem list_items[], const size_t size, const size_t current_index)
{
  for (size_t i = 0; i < size; ++i)
  {
    if (i > 4) // ListBox can only hold 5 items
    {
      break;
    }
    if (i == current_index)
    {
      screen->setPrintPos(pos_x, pos_y + i * 10);
      screen->print("-");
    }
    screen->setPrintPos(pos_x + 5, pos_y + i * 10);
    screen->print(list_items[i].text);
    screen->drawLine(0, pos_y + i * 10 + 9, WIDTH - 1, pos_y + i * 10 + 9);
  }
}

void mUI::drawPopup(const char *message, const uint16_t pos_x, const uint16_t pos_y)
{
  char *line;
  char msg_temp[50];
  strcpy(msg_temp, message);
  uint8_t lineno = 0;

  // Clear area
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(pos_x, pos_y, 120, 28);

  screen->setColor(0, GREEN); // Main Color: Green
  screen->drawFrame(pos_x, pos_y, 120, 28);

  line = strtok(msg_temp, "\n");
  while (line != nullptr)
  {
    screen->setPrintPos(pos_x + 5, pos_y + 5 + lineno * 10);
    screen->print(line);
    line = strtok(nullptr, "\n");
    lineno++;
  }
}

// void mUI::drawWindow(Window &window)
// {
//   drawMenuFrame(window.title);
//   for (uint8_t i = 0; i < window.num_of_widgets; i++)
//   {
//     switch (window.widgets[i].type)
//     {
//     case WidgetType::LISTBOX:
//       if (window.widgets[i].draw_needed)
//         drawListBox(window.widgets[i].pos_x, window.widgets[i].pos_y, window.widgets[i].data.list_box.items, window.widgets[i].data.list_box.size, window.widgets[i].data.list_box.list_index);
//       break;

//     case WidgetType::BUTTON:
//       if (window.widgets[i].draw_needed)
//         drawButton(window.widgets[i].data.button.index, window.widgets[i].pos_x, window.widgets[i].pos_y, window.widgets[i].width, window.widgets[i].height, window.widgets[i].data.button.text);
//       break;

//     case WidgetType::LABEL:
//       if (window.widgets[i].draw_needed)
//         drawText(window.widgets[i].pos_x, window.widgets[i].pos_y, window.widgets[i].data.label.text);
//       break;

//     default:
//       break;
//     }
//   }
// }
#endif // !USE_U8G2