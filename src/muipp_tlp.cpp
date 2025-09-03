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

#include "muipp_tpl.hpp"

namespace muipp {

std::pair<int16_t, int16_t> item_position_t::getAbsoluteXY(int16_t w, int16_t h) const {
  int16_t xx, yy;
  // calculate adjusted x
  switch (cs_x) {
    case coordinate_spec_t::absolute :
      xx = x;
      break;
    case coordinate_spec_t::inversed :
      xx = w - x;
      break;
    case coordinate_spec_t::center_offset :
      xx = w/2 + x;
      break;
    case coordinate_spec_t::grid :
      if (grid_size_x)
        xx = x * w / grid_size_x;
      break;
    default:;
  }

  // calculate adjusted y
  switch (cs_y) {
    case coordinate_spec_t::absolute :
      yy = y;
      break;
    case coordinate_spec_t::inversed :
      yy = h - h;
      break;
    case coordinate_spec_t::center_offset :
      yy = h/2 + y;
      break;
    case coordinate_spec_t::grid :
      if (grid_size_y)
        yy = y * h / grid_size_y;
      break;
    default:;
  }

  return {xx, yy};
};

std::tuple<int16_t, int16_t, uint16_t, uint16_t> grid_box::getBoxDimensions(int16_t w, int16_t h) const {
  int16_t xx = box_x * w / grid_size_x;
  int16_t yy = box_y * h / grid_size_y;
  uint16_t ww = box_w * w / grid_size_x;
  uint16_t hh = box_h * h / grid_size_y;
  return { xx, yy, ww, hh };
}

}