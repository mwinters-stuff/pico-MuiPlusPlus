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

#pragma once
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "muipp_u8g2.hpp"
#include "literals.h"
#include <Versatile_RotaryEncoder.h>
#include <u8g2.h>
#include <memory>
#include <string>


/*
    this class controls our buttons and
    switch display information between some regular jobs and Configuration menu.
    It's nearly same functionality as in previous example, just wrapped into class object
*/
class DisplayControls {
private:
  Versatile_RotaryEncoder encoder;
  u8g2_t u8g2;
  // screen refresh required flag
  bool _rr{true};

  // in-Menu state flag
  bool _inMenu{false};

  // a placeholder for our MuiPlusPlus menu object, initially empty
  std::unique_ptr<MuiPlusPlus> _menu;

  /**
   * @brief this method I'll call when my "enter" button is pressed and I need to pass "enter"
   * event to menu and receive reply event from menu to understand when menu has exited
   * 
   */
  void _menu_ok_action();

  /**
   * @brief this method I'll call when my "+/-" buttons are pressed and I need to pass "cursor"
   * event to menu and receive reply event from menu to understand when menu has exited
   * 
   */
  void _menu_encoder_action(int8_t rotation);

  // menu builder function
  void _buildMenu();

  // Rotary encoder callbacks
  void handleRotate(int8_t rotation);
  void handlePressRelease();
  void handleLongPressRelease();
public:
  // constructor
  DisplayControls();

  // destructor
  ~DisplayControls();

  // start our display control
  void begin();

  // draw something on screen, either some silly stub text, or render menu
  void drawScreen();
};

