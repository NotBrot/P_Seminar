/*
 *
 */

#include "mUI.h"

#ifdef USE_U8G2
void mUI::begin(U8G2 *display)
{
  screen = display;
  screen->setFont(u8g2_font_5x8_mr);
  screen->setFontPosTop(); // Set reference position for chars to top-left
}

void mUI::clearScreen()
{
  screen->clearBuffer();
  screen->clearDisplay();
}

void mUI::drawMenuFrame(const char *title)
{
  screen->setFont(u8g2_font_5x8_mr);

  screen->drawFrame(0, 10, WIDTH, HEIGHT - 10);

  screen->setCursor(0, 0);
  screen->print(title);

  // screen->drawLine(0, 20, WIDTH - 1, 20);
}

void mUI::drawStatus(const char status)
{
  screen->setCursor(WIDTH - 6, 1);
  screen->print(status);
}

void mUI::drawButton(const char index, const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text)
{
  // Clear old button
  screen->setDrawColor(0);
  screen->drawBox(pos_x, pos_y, w, h);

  screen->setDrawColor(1);
  screen->drawFrame(pos_x, pos_y, w, h);

  if (current_button_index == index)
    screen->drawFrame(pos_x + 2, pos_y + 2, w - 4, h - 4);

  screen->setCursor(pos_x + 3, pos_y + 3);
  screen->print(text);
}

void mUI::drawText(const uint16_t pos_x, const uint16_t pos_y, const char *text)
{
  // screen->setCursor(pos_x + 3, pos_y + 3);
  // screen->print(text);
  screen->drawStr(pos_x + 3, pos_y + 3, text);
}

void mUI::drawListBox(const uint16_t pos_x, const uint16_t pos_y, const char *list_items[], const size_t size, const size_t current_index)
{
  for (size_t i = 0; i < size; ++i)
  {
    if (i > 4) // ListBox can only hold 5 items
    {
      break;
    }
    if (i == current_index)
    {
      screen->setCursor(pos_x, pos_y + i * 10);
      screen->print("-");
    }
    screen->setCursor(pos_x + 5, pos_y + i * 10);
    screen->print(list_items[i]);
    screen->drawLine(0, pos_y + i * 10 + 9, WIDTH - 1, pos_y + i * 10 + 9);
  }
}

void mUI::drawPopup(const char* message, const uint16_t pos_x, const uint16_t pos_y)
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
    screen->setCursor(pos_x + 5, pos_y + 5 + lineno * 10);
    screen->print(line);
    line = strtok(nullptr, "\n");
    lineno++;
  }
}

void mUI::drawWindow(Window *window)
{
  drawMenuFrame(window->title);
  for (uint8_t i = 0; i < window->num_of_widgets; i++)
  {
    switch (window->widgets[i].type)
    {
    case WidgetType::BUTTON:
      if (window->widgets[i].has_changed)
      {
        drawButton(window->widgets[i].index, window->widgets[i].pos_x, window->widgets[i].pos_y, window->widgets[i].width, window->widgets[i].has_changed, window->widgets[i].text);
      }
      break;

    case WidgetType::LABEL:
      break;

    default:
      break;
    }
  }
}
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

void mUI::drawButton(const char index, const uint16_t pos_x, uint16_t pos_y, uint16_t w, uint16_t h, const char *text)
{
  // Clear old button
  screen->setColor(0, BLACK); // Main Color: Black
  screen->drawBox(pos_x, pos_y, w, h);

  screen->setColor(0, GREEN); // Main Color: Green
  screen->drawFrame(pos_x, pos_y, w, h);

  if (current_button_index == index)
    screen->drawFrame(pos_x + 2, pos_y + 2, w - 4, h - 4);

  screen->setPrintPos(pos_x + 3, pos_y + 3);
  screen->print(text);
}

void mUI::drawText(const uint16_t pos_x, const uint16_t pos_y, const char *text)
{
  screen->setPrintPos(pos_x + 3, pos_y + 3);
  screen->print(text);
}

void mUI::drawListBox(const uint16_t pos_x, const uint16_t pos_y, const char *list_items[], const size_t size, const size_t current_index)
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
    screen->print(list_items[i]);
    screen->drawLine(0, pos_y + i * 10 + 9, WIDTH - 1, pos_y + i * 10 + 9);
  }
}

void mUI::drawPopup(const char* message, const uint16_t pos_x, const uint16_t pos_y)
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

void mUI::drawWindow(Window *window)
{
  drawMenuFrame(window->title);
  for (uint8_t i = 0; i < window->num_of_widgets; i++)
  {
    switch (window->widgets[i].type)
    {
    case WidgetType::BUTTON:
      if (window->widgets[i].has_changed)
      {
        drawButton(window->widgets[i].index, window->widgets[i].pos_x, window->widgets[i].pos_y, window->widgets[i].width, window->widgets[i].has_changed, window->widgets[i].text);
      }
      break;

    case WidgetType::LABEL:
      break;

    default:
      break;
    }
  }
}
#endif // !USE_U8G2