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

#if __has_include("Arduino_GFX.h")
#include "muipp_agfx.hpp"

using namespace muipp;

void MuiItem_AGFX_GenericTXT::alignText(Arduino_GFX* g, int16_t w, int16_t h, muipp::text_align_t halign, muipp::text_align_t valign, const char* text){
  int16_t xadj, xx, yy;
  uint16_t ww, hh;
  // find horizontal position for cursor
  switch (halign){
    // try to align text centered to defined canvas width
    case text_align_t::center : {
      g->getTextBounds(text, g->getCursorX(), g->getCursorY(), &xx, &yy, &ww, &hh);
      xadj = g->getCursorX() - ww/2;
    }
      break;
    // calculate left offset from the specified x position to fit the string
    case text_align_t::right :
      g->getTextBounds(text, g->getCursorX(), g->getCursorY(), &xx, &yy, &ww, &hh);
      xadj = g->getCursorX() - ww;
      break;
    default:
      xadj = g->getCursorX();
  }
  g->setCursor(xadj, g->getCursorY());  // y offset is not implemented yet
}

void MuiItem_AGFX_StaticText::render(const MuiItem* parent, void* r){
  if (!getName()) return; // do not print NULL
  Arduino_GFX* g = static_cast<Arduino_GFX*>(r);
  // clear area from a previous text
  g->fillRect(xx, yy, ww, hh, cfg.bgcolor);
  if (cfg.transp_bg)
    g->setTextColor(cfg.color);
  else {
    g->setTextColor(cfg.color, cfg.bgcolor);
  }

  g->setCursor(_x, _y);
  g->setFont(cfg.font);
  g->setTextSize(1);    // reset to default
  alignText(g, getName());
  g->getTextBounds(getName(), g->getCursorX(), g->getCursorY(), &xx, &yy, &ww, &hh);
  g->print(getName());
  refresh = false;
}


void MuiItem_AGFX_TextCallBack::render(const MuiItem* parent, void* r){
/*
  if (_font)
    static_cast<Arduino_GFX*>(r)->setFont(_font);

  static_cast<Arduino_GFX*>(r)->setCursor(_font);

  auto a = getXoffset(name);
  static_cast<Arduino_GFX*>(r)->print(a, _y, _cb());
*/
  }
  
//  ******************
//  CanvasTextScroller
//  ******************

void CanvasTextScroller::begin(const char* text, const uint8_t* font, uint8_t font_size){
  if (!strlen(text)) return;
  _text = text;
  _c.setCursor(0, _c.height());
  _c.setFont(font);
  _c.setTextSize(font_size);
  _c.setTextWrap(false);
  _c.getTextBounds(_text, 0, _c.height(), &xx, &yy, &ww, &hh);
  _xPos = _c.width(); // Start from the right edge
  _lastUpdate = millis();
  _inactive = false;
}

void CanvasTextScroller::update(const char* text){
  if (!strlen(text)) return;
  _text = text;
  _c.getTextBounds(_text, 0, _c.height(), &xx, &yy, &ww, &hh);
}

bool CanvasTextScroller::scroll_pending() const {
  if (!_text) return false;
  unsigned long now = millis();
  unsigned long elapsed = now - _lastUpdate;
  float newPos = _xPos - _speed * elapsed;
  // find if time for one px shift has passed
  return (_xPos - newPos >= 1.0);
}

bool CanvasTextScroller::scroll() {
  if (!_text) return false;
  // check if scroller was aborted asynchronously
  if (_inactive){
    _text = NULL;
    if (_cb)
      _cb(event_t::end);
  }

  unsigned long now = millis();
  unsigned long elapsed = now - _lastUpdate;
  float newPos = _xPos - _speed * elapsed;
  // find if time for one px shift has passed
  if (_xPos - newPos < 1.0 )
    return false;

  // make an event call if needed
  if (_cb){
    if (newPos < 0 && _xPos > 0 && _cb(event_t::head_at_left))                                    // check if head reached right edge
      return false;
    else if (_xPos + ww > _c.width() && newPos + ww < _c.width() && _cb(event_t::tail_at_right))  // check if tail reached right edge
      return false;
    else if (_xPos + ww > 0 && newPos + ww < 0 && _cb(event_t::tail_at_left))                      // check if tail reached left edge
      return false;
  }

  // If text is completely off-screen to the left, reset it to right edge
  _xPos = newPos + ww < 0 ? _c.width() : newPos;
  // check if string was rolled over, then generate an event on this
  if (_cb && (_xPos == _c.width()) && _cb(event_t::head_at_left))
    return false;

  _drawall();
  //_drawVisible();
  _lastUpdate = now;
  return true;
}


void CanvasTextScroller::_drawall() {
  _c.fillScreen(0);
  // calculate offset from a baseline for current text
  _c.setCursor(_xPos,_c.height() - (yy + hh - _c.height()));
  _c.print(_text);
}
/*
void CanvasTextScroller::_drawVisible() {
  _c.fillScreen(0);

  float cursorX = _xPos;
  //Serial.printf("t:%f\n", _xPos);

  // iterate by chars, print only those chars that are visible on canvas
  for (size_t i = 0; i < _text.length(); i++) {
    char c = _text[i];
    int16_t x1, y1;
    uint16_t w, h;

    _c.getTextBounds(String(c), cursorX, yy, &x1, &y1, &w, &h);    // this String is damn ugly, but Arduino_GFX does not have getTextBounds() for a char, agrrr....

    if (x1 + w >= 0 && x1 <= _c.width()) {
      _c.setCursor(cursorX, _c.height()/2);
      _c.print(c);
    }

    cursorX += w; // advance cursor manually by character width
    if (cursorX > _c.width()) break; // beyond screen
  }
}
*/


#endif  // __has_include("Arduino_GFX.h")