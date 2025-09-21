/*
  MuiPlusPlus example

  Copyright (C) Emil Muratov, 2024
  GitHub: https://github.com/vortigont/MuiPlusPlus

 *  This program or library is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *  Public License version 2 for more details.
 *
 *  You should have received a copy of the GNU General Public License version 2
 *  along with this library; if not, get one at
 *  https://opensource.org/licenses/GPL-2.1
 */

#include "muimenu.hpp"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "main.h"
#include "pico/stdlib.h"
#include "u8g2functions.h"
#include <Versatile_RotaryEncoder.h>
#include <u8g2.h>

// rotatry encoder pins
#define ROTARY_clk 20
#define ROTARY_dt 21
#define ROTARY_sw 19

// let's select some fonts for our Menu
#define MAIN_MENU_FONT u8g2_font_bauhaus2015_tr
#define SMALL_TEXT_FONT u8g2_font_glasstown_nbp_t_all

// messages for non-menu display operations
const char *incr = "incr button";
const char *decr = "decr button";
const char *ok = "ok button";
const char *anyk = "Press any key";
const char *quitmenu = "menu closed";

const char *stub_text = anyk;

// Constructor, will create buttons object members, assigning pins to it
DisplayControls::DisplayControls() : encoder(ROTARY_clk, ROTARY_dt, ROTARY_sw) {
  // u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay_hw_i2c);
  u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay_hw_i2c);
}

/**
 * @brief this method will setup our DisplayControl class and begins its operation
 * 
 */

void DisplayControls::handleRotate(int8_t rotation) {
  if (_inMenu && _menu)
    _menu_encoder_action(rotation);
  else {
    // otherwise, when I'm not in menu, just change a message displayed on screen
    if (rotation == Versatile_RotaryEncoder::Rotary::right) {
      stub_text = decr;
    } else if (rotation == Versatile_RotaryEncoder::Rotary::left) {
      stub_text = incr;
    }
}
  // redraw screen
  _rr = true;
}

void DisplayControls::handlePressRelease() {
  if (_inMenu && _menu) {
    _menu_ok_action();
  } else {
    stub_text = ok;
    // printf("OK Click\n");
  }
  _rr = true;
}

void DisplayControls::handleLongPressRelease() {
  if (!_inMenu) {
    // currenlty I'm not in menu, so I need to dynamically create menu object from here and
    // let it drive the display with menu functions
    // for this I'm creating an instance of MuiPlusPlus object
    // so the idea is - Menu Object is instantiated in memory ONLY when I'm entering menu and released on quit
    _menu = std::make_unique<MuiPlusPlus>();
    _buildMenu();
    // set the flag, indicating that now I have menu object created for this same _evt_button() function would know to redirect further "OK" keypresses to menu from now on
    _inMenu = true;
  } else {
    // we are already in Menu,
    // so I'm sending 'escape' mui_event there to _menu object, and save what event I receive in return to this press
    // let's save it as 'e' object (a mui_event structure)
    auto e = _menu->muiEvent(mui_event(mui_event_t::escape));

    // Now I need to check if I received a reply with 'quitMenu' event back from menu object
    // if that is so then I need to switch to Main Work Screen since menu has exited, long press will always quit Menu to main screen
    if (e.eid == mui_event_t::quitMenu) {
      // release our menu object - i.e. destruct it, releasing all memory
      if (_menu)
        _menu.release();
      // set flag to indicate we are no longer in menu
      _inMenu = false;
      // change a message we print on a screen
      stub_text = quitmenu;
      // printf("menu object destroyed\n");
    }
  }
  _rr = true;
}

void DisplayControls::begin() {
  encoder.setHandleRotate([this](int8_t rotation) { handleRotate(rotation); });
  encoder.setHandlePressRelease([this]() { handlePressRelease(); });
  encoder.setHandleLongPressRelease([this]() { handleLongPressRelease(); });

  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_SendBuffer(&u8g2);
}

/**
 * @brief Destroy the Display Controls object
 */
DisplayControls::~DisplayControls() {
  // Disable GPIO interrupts
}


/**
 * @brief this method I'll call when my "enter" button is pressed and I need to pass "enter"
 * event to menu and receive reply event from menu to understand when menu has exited
 * 
 */
void DisplayControls::_menu_ok_action() {
  auto e = _menu->muiEvent(mui_event(mui_event_t::enter));
  if (e.eid == mui_event_t::quitMenu) {
    _menu.release();
    _inMenu = false;
    stub_text = quitmenu;
    // printf("menu object destroyed\n");
  }
}

void DisplayControls::_menu_encoder_action(int8_t rotation) {
  // we are in menu, let's send "encoder" events to menu
  // I do not care about returned events from menu for encoder buttons presses for now because I know that
  // menu could quit only on "OK" button press or longPress
  if (rotation == Versatile_RotaryEncoder::Rotary::right) {
    _menu->muiEvent(mui_event(mui_event_t::moveUp));
  } else if (rotation == Versatile_RotaryEncoder::Rotary::left) {
    _menu->muiEvent(mui_event(mui_event_t::moveDown));
  }
}


/**
 * @brief this method will render screen
 * when in menu it calls MuiPlusPlus object to do its job
 * when not in menu it just prints stub text messages
 * 
 */
void DisplayControls::drawScreen() {
  encoder.ReadEncoder();
  if (!_rr) return;

  u8g2_ClearBuffer(&u8g2);
  if (_inMenu && _menu) {
    // printf("Render menu:%lu ms\n", to_ms_since_boot(get_absolute_time()));
    _menu->render();
  } else {
    // printf("Render welcome screen\n");
    u8g2_SetFont(&u8g2, SMALL_TEXT_FONT);
    u8g2_DrawStr(&u8g2, 0, u8g2_GetDisplayHeight(&u8g2) / 2, stub_text);
  }
  u8g2_SendBuffer(&u8g2);

  _rr = false;
}

/**
 * @brief this method will build our menu
 * in this example it would be also very simple - a just a scroll list of menu items to display
 * let's assume we have following structure
 * 
 * 'root menu title'
 *  |- Temperature
 *  |     |- Some stub page
 *  |- Timeouts
 *  |     |- Some stub page
 *  |- Tip
 *  |     |- Some stub page
 *  |- Power Supply
 *  |     |- Some stub page
 *  |- Information
 *  |     |- Some stub page
 *  |- <Back
 * 
 */
void DisplayControls::_buildMenu() {
  muiItemId root_page = _menu->makePage(lang_en_us::T_Settings);
  _menu->addMuippItem(new MuiItem_U8g2_PageTitle(u8g2, _menu->nextIndex(), PAGE_TITLE_FONT_SMALL), root_page);

  muiItemId scroll_list_id = _menu->nextIndex();
  auto list = new MuiItem_U8g2_DynamicScrollList(
      u8g2,
      scroll_list_id,
      [](size_t index) { return menu_MainConfiguration.at(index); },
      []() { return menu_MainConfiguration.size(); },
      nullptr,
      MAIN_MENU_Y_SHIFT, MAIN_MENU_ROWS,
      MAIN_MENU_X_OFFSET, MAIN_MENU_Y_OFFSET,
      MAIN_MENU_FONT3, MAIN_MENU_FONT3);

  list->listopts.page_selector = true;
  list->listopts.back_on_last = true;
  list->on_escape = mui_event_t::quitMenu;

  _menu->addMuippItem(list, root_page);
  _menu->pageAutoSelect(root_page, scroll_list_id);

  muiItemId title2_id = _menu->nextIndex();
  _menu->addMuippItem(new MuiItem_U8g2_PageTitle(u8g2, title2_id, PAGE_TITLE_FONT));

  muiItemId quit_idx = _menu->nextIndex();
  auto quitbtn = new MuiItem_U8g2_ActionButton(
      u8g2,
      quit_idx,
      mui_event_t::escape,
      "Return back",
      SMALL_TEXT_FONT,
      u8g2_GetDisplayWidth(&u8g2) / 2,
      u8g2_GetDisplayHeight(&u8g2) / 2,
      muipp::text_align_t::center,
      muipp::text_align_t::bottom);

  _menu->addMuippItem(quitbtn, quit_idx);

  for (auto i = 0; i != (menu_MainConfiguration.size() - 1); ++i) {
    muiItemId page = _menu->makePage(menu_MainConfiguration.at(i), root_page);
    _menu->addItemToPage(title2_id, page);
    _menu->addItemToPage(quit_idx, page);
    _menu->pageAutoSelect(page, quit_idx);
  }

  _menu->menuStart(root_page);
}
