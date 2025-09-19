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
#include <functional>
#include <string_view>

using muiItemId = uint32_t;

namespace muipp {

// callback function that returns index size
using size_cb_t = std::function< size_t (void)>;
// callback function that accepts index value
using index_cb_t = std::function< void (size_t index)>;
// callback that just returns string
using string_cb_t = std::function< const char* (void)>;
// callback function that accepts index and returns const char* string associated with index
using stringbyindex_cb_t = std::function< const char* (size_t index)>;
// callback function for constrained numeric
template <typename T>
using constrain_val_cb_t = std::function< void (muiItemId id, T value, T min, T max, T step)>;
// stringifying function, it accepts some object value and returns a string that identifies the value (i.e. convert int to asci, etc...)
template <typename T>
using stringify_cb_t = std::function< std::string (T value)>;

// callback function that returns something by value :) mostly usefull with POD objects - ints, floats, etc...
template <typename T>
using value_cb_t = std::function< T (void)>;

/**
 * @brief text alignment specifier
 * applicable mostly to U8g2 since it has best font positioning support,
 * but also could be used for other objects
 */
enum class text_align_t {
  baseline = 0,
  center,
  top,
  bottom,
  left,
  right
};

/**
 * @brief specifier for placemnent coordinates
 * used in 'item_position_t' strunct
 * 
 */
enum class coordinate_spec_t {
  absolute = 0,   // absolute position, i.e. x,y conted from the top left corner of the display
  //relative,       // pixel offset from previous item (if applicable)
  inversed,       // coordinate specifies offset from the opposite border of the screen, i.e. for x = 10 means "offset 10 px left from the right edge"
  center_offset,  // offset from a center of axis, width ot height
  grid            // treat x,y as grid coordinates, where base for the grid is defined via additional denominators
};

/**
 * @brief struct defines item positioning
 * could be used for absolute or relative positioning
 * 
 */
struct item_position_t {
  // placement values x, y axis
  int16_t x, y;
  // coordinate specifier (how to treat x and y)
  coordinate_spec_t cs_x, cs_y;
  // denominator for proportional coordinates
  uint8_t grid_size_x, grid_size_y;

  /**
   * @brief calculates absolute position for specified canvas with dimentions WxH
   * 
   */
  std::pair<int16_t, int16_t> getAbsoluteXY(int16_t w, int16_t h) const;
};


struct grid_box {
  // grid size
  uint16_t grid_size_x, grid_size_y;
  // position on a grid
  int16_t box_x, box_y;
  // box size in grid's units
  uint16_t box_w, box_h;
  std::tuple<int16_t, int16_t, uint16_t, uint16_t> getBoxDimensions(int16_t w, int16_t h) const;
};


// Unary predicate for Mui's label search matching
template <class T>
class MatchLabel {
  std::string_view _lookup;
public:
  explicit MatchLabel(const char* label) : _lookup(label) {}
  constexpr bool operator() (const T& item ){
      // T is MuiPage
      return _lookup.compare(item.getName()) == 0;
  }
};

// Unary predicate for Mui's ID search matching
template <class T>
class MatchID {
  muiItemId _id;
public:
  explicit MatchID(muiItemId id) : _id(id) {}
  constexpr bool operator() (const T& item ){
      // T is MuiItem_pt
      return item->id == _id;
  }
};

// Unary predicate for Mui's ID search matching
template <class T>
class MatchPageID {
  muiItemId _id;
public:
  explicit MatchPageID(muiItemId id) : _id(id) {}
  constexpr bool operator() (const T& item ){
      // T is MuiPage
      return item.id == _id;
  }
};

// a simple constrain function
template<typename T>
T clamp(T value, T min, T max){
  return (value < min)? min : (value > max)? max : value;
}

} // end of namespace muipp
#endif