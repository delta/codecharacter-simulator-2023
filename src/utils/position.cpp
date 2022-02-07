#include "utils/position.hpp"

#include <algorithm>
#include <cmath>

#include "utils/game_map.hpp"

Position::Position(int x, int y)
    : _x(std::clamp(x, 0, (int)Map::no_of_cols)),
      _y(std::clamp(y, 0, (int)Map::no_of_rows)) {}

int Position::get_x() const { return this->_x; }

int Position::get_y() const { return this->_y; }

double Position::distance_to(Position other) const {
  auto delta_x = other.get_x() - this->get_x();
  auto delta_y = other.get_y() - this->get_y();
  return sqrt((double)(delta_x * delta_x + delta_y * delta_y));
}
