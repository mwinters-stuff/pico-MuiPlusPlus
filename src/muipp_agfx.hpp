/*
    This file is a part of MuiPlusPlus project
    https://github.com/vortigont/MuiPlusPlus

    Copyright © 2024-2025 Emil Muratov (vortigont)

    MuiPlusPlus is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MuiPlusPlus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MuiPlusPlus.  If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once
#include <string>
#include "Arduino_GFX.h"
#include "canvas/Arduino_Canvas_Mono.h"
#include "muiplusplus.hpp"

/**
 * @brief aggregate holds options to print text at some pont
 * 
 */
struct AGFX_text_t {
  const uint8_t* font{nullptr};         // U8G2 font
  uint16_t color, bgcolor;              // font / background color
  uint8_t font_size;                    // fint size multiplicator
  muipp::text_align_t halign{muipp::text_align_t::left}, valign{muipp::text_align_t::baseline};   // text alignment
  bool transp_bg{true};                 // transparent background
};

/**
 * @brief ArduinoGFX lib's generic item
 * 
 */
class MuiItem_AGFX_GenericTXT {
protected:
  // item's initial cursor position
  int16_t _x, _y;
  // text printing options
  AGFX_text_t cfg;

public:

  /**
   * @brief Construct a new MuiItem_U8g2_PageTitle object
   * generic object is NOT selectable!
   * @param id assigned id for the item
   * @param font use font for printing, if null, then do not switch font
   * @param x, y Coordinates of the top left corner to start printing
   */
  MuiItem_AGFX_GenericTXT(int16_t x = 0, int16_t y = 0, const AGFX_text_t& tcfg = {}) : _x(x), _y(y), cfg(tcfg) {};

  MuiItem_AGFX_GenericTXT(std::pair<int16_t, int16_t> xy, const AGFX_text_t& tcfg = {}) : _x(xy.first), _y(xy.second), cfg(tcfg) {};

  int16_t getX() const { return _x; }

  int16_t getY() const { return _y; }

  // adjust cursor position
  void setCursor( int16_t x, int16_t  y){ x = _x; _y = y; }

  // adjust text alignment
  void setTextAlignment(muipp::text_align_t hAlign, muipp::text_align_t vAlign){ cfg.halign = hAlign; cfg.valign = vAlign; }

  /**
   * @brief adjusted current cursor's position to print provided text acording to text alignment parameters (v_align h_align)
   * current cursor postion is considered to be at lower left corner of the area to print text to
   * @note this call must be followed by text 'print' call
   * 
   * @param text 
   * 
   */
  void alignText(Arduino_GFX* g, int16_t w, int16_t h, muipp::text_align_t halign, muipp::text_align_t valign, const char* text);
  void alignText(Arduino_GFX* g, int16_t w, int16_t h, const char* text){ alignText(g, w, h, cfg.halign, cfg.valign, text); };
  void alignText(Arduino_GFX* g, const char* text){ alignText(g, g->width(), g->height(), cfg.halign, cfg.valign, text); };
};

/**
 * @brief displays static text at specific place
 * @note prints a text by pointer! Text must be persistent in memory (intended to print strings from ROM)
 * 
 */
class MuiItem_AGFX_StaticText : public MuiItem_AGFX_GenericTXT, public MuiItem_Uncontrollable {
  // text block
  int16_t  xx{0}, yy{0};
  uint16_t ww{0}, hh{0};

public:
  /**
   * @brief Construct a new MuiItem_U8g2_PageTitle object
   * 
   * @param u8g2 refernce to display object
   * @param id assigned id for the item
   * @param font use font for printing, if null, then do not switch font
   * @param x, y Coordinates of the top left corner to start printing
   */
  MuiItem_AGFX_StaticText(muiItemId id, const char* text, u8g2_uint_t x = 0, u8g2_uint_t y = 0, const AGFX_text_t& tcfg = {})
    : MuiItem_AGFX_GenericTXT(x, y, tcfg),
      MuiItem_Uncontrollable(id, text) {};

  MuiItem_AGFX_StaticText(muiItemId id, const char* text, std::pair<int16_t, int16_t> xy, const AGFX_text_t& tcfg = {})
  : MuiItem_AGFX_GenericTXT(xy, tcfg),
    MuiItem_Uncontrollable(id, text) {};
  
  void render(const MuiItem* parent, void* r = nullptr) override;
};
  

class MuiItem_AGFX_TextCallBack : public MuiItem_AGFX_GenericTXT, public MuiItem_Uncontrollable {
  muipp::string_cb_t _cb;
public:
  /**
   * @brief Construct a new MuiItem_U8g2_PageTitle object
   * 
   * @param id assigned id for the item
   * @param callback - callbak that returns text to draw
   * @param x, y Coordinates of the top left corner to start printing
   * @param font use font for printing, if null, then do not switch current font
   */
  MuiItem_AGFX_TextCallBack(muiItemId id, muipp::string_cb_t callback,
      int16_t x = 0, int16_t y = 0,
      AGFX_text_t tcfg = {})
        : MuiItem_AGFX_GenericTXT(x, y, tcfg),
          MuiItem_Uncontrollable(id), _cb(callback) {};

  MuiItem_AGFX_TextCallBack(muiItemId id, muipp::string_cb_t callback,
    std::pair<int16_t, int16_t> xy,
    AGFX_text_t tcfg = {})
      : MuiItem_AGFX_GenericTXT(xy, tcfg),
        MuiItem_Uncontrollable(id), _cb(callback) {};
      

  void render(const MuiItem* parent, void* r = nullptr) override;
};


/**
 * @brief text scroller that uses binary canvas to render text 
 * 
 */
class CanvasTextScroller {
public:
  CanvasTextScroller(uint16_t w, uint16_t h) : _c(w, h, nullptr) { _c.begin(); _c.setUTF8Print(true); }    // canvas will malloc in c-tor

  enum class event_t { 
    head_at_left,     // head of the string reached left edge of the canvas, i.e. head start going off the screen if moved from right to left
    head_at_right,    // head of the string reached right edge of the canvas, i.e. head just started moving from right to left
    tail_at_right,    // tail of the string reached right edge of the canvas
    tail_at_left,     // tail of the string reached left edge of the canvas, i.e. string got out of the visibility
    end               // scrolling complete, aborted, etc... on this event the pointer to text string is invalidated. A new scrolling should be set with begin()
  };

  /**
   * @brief callback function for scrolling events
   * if function returns true - abort current render and wait for the next call, could be used to reload the scroll text when reaching certain portions of the screen
   * if function returns false - keep rendering the text
   */
  using event_cb = std::function< bool (event_t e)>;

  /**
   * @brief begin scrolling text
   * 
   * @param text the pointer MUST persist during whole scrolling duration!
   * @note do NOT pass here temporary created String's, etc...
   * @note begin() is NOT thread safe if scroll() is executed in another thread! If using from different threads, call abort() first then assign new text pointer from a callback
   * @param font U8G2 unicode font
   * @param font_size font scaling
   */
  void begin(const char* text, const uint8_t* font, uint8_t font_size = 1);

  /**
   * @brief Update currently scrolled text
   * @note update() is NOT thread safe if scroll() is executed in another thread! If using from different threads, call abort() first then assign new text pointer from a callback
   * 
   * @param text 
   */
  void update(const char* text);

  /**
   * @brief asynchronously abort scrolling
   * @note instance will release text pointer only on a next scroll() call!!!
   * should be used along with callbacks for thread-safe cases, otherwise just use begin()
   * @note a new scrolling must be set with begin()
   * 
   */
  void abort(){ _inactive = true; };

  // set scrolling speed in pixels per second
  void setSpeed(uint32_t v){ _speed = v / 1000.0; };

  /**
   * @brief checks if scroll redraw is peding
   * if true, than a subsequent call to scroll() is required to actually render the text
   * 
   */
  bool scroll_pending() const;

  /**
   * @brief render text on canvas
   * 
   * @return true if rendereng was done, i.e. elapset time from the last call required to move text at least for 1 pixel
   * @return false if no rendering was done and canvas content left intacted
   */
  bool scroll();

  // set event callback
  void setCallBack(event_cb f){ _cb = f; }

  // access the canvas
  const uint8_t* getFramebuffer(){ return _c.getFramebuffer(); }

  // reset current scroller position, start the text from right edge
  void reset(){ _xPos = _c.width(); _lastUpdate = millis(); };

  int16_t getW() const { return _c.width(); }
  int16_t getH() const { return _c.height(); }

protected:
  Arduino_Canvas_Mono _c;

private:
  float _speed;    // pixels per ms
  const char* _text{NULL};
  bool _inactive{false};

  // callback function
  event_cb _cb;

  // full text block dimensions
  int16_t  xx, yy;
  uint16_t ww, hh;

  float _xPos;   // current X position
  unsigned long _lastUpdate;

  /**
   * @brief optimized drawing only visibile chars
   * @note does not work properly with unicode
   * 
   */
  //void _drawVisible();

  void _drawall();
};

/**
 * @brief class wraps text scroller and MuiPP to control positioning and rendering style
 * 
 */
class AGFX_TextScroller : public MuiItem_Uncontrollable, public CanvasTextScroller {
  int16_t _x, _y;
  AGFX_text_t _tcfg;
public:
  /**
   * @brief text scroller via ArduinoGFX's canvas
   * 
   * @param id assigned id for the item
   * @param x, y Coordinates of the top left corner to start printing
   * @param w, h canvas size where srolled text will be printed
   * @param speed - speed of scrolling, pixel per second
   * @param tcfg text decoration config
   */
  AGFX_TextScroller(muiItemId id,
      int16_t x, int16_t y,
      uint16_t w, uint16_t h,
      const AGFX_text_t& tcfg,
      float speed = 25,
      const char* label = NULL)
        : MuiItem_Uncontrollable(id, label), CanvasTextScroller(w, h), _x(x), _y(y), _tcfg(tcfg) { setSpeed(speed); };

  AGFX_TextScroller(muiItemId id,
      std::tuple<int16_t, int16_t, uint16_t, uint16_t> dim,
      const AGFX_text_t& tcfg,
      float speed = 25,
      const char* label = NULL)
        : MuiItem_Uncontrollable(id, label), _x(std::get<0>(dim)), _y(std::get<1>(dim)), CanvasTextScroller(std::get<2>(dim), std::get<3>(dim)), _tcfg(tcfg) { setSpeed(speed); };

  // begin text scrolling with predefined font settings
  void begin(const char* text){ CanvasTextScroller::begin(text, _tcfg.font, _tcfg.font_size); };
  void render(const MuiItem* parent, void* r = nullptr) override { if (scroll()) static_cast<Arduino_GFX*>(r)->drawBitmap(_x, _y, _c.getFramebuffer(), getW(), getH(), _tcfg.color, _tcfg.bgcolor); };
  bool refresh_req() const override { return scroll_pending(); };
};

class MuiItem_RangeSlider : public MuiItem {

  muipp::grid_box _pos;

public:
  MuiItem_RangeSlider(muiItemId id, const char* text, const muipp::grid_box& position) : MuiItem(id) {}

};
