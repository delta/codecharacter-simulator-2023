#pragma once

#include <compare>

enum class PvpPlayerType {
  PLAYER1,
  PLAYER2,
};
class Position {
private:
  int _x;
  int _y;

public:
  Position(int x, int y);
  [[nodiscard]] int get_x() const;
  [[nodiscard]] int get_y() const;
  [[nodiscard]] double distance_to(Position other) const;
  auto operator<=>(const Position &other) const = default;
  static bool is_valid_spawn_position(int, int);
  static bool is_valid_pvp_spawn_position(int, int, PvpPlayerType player_type);
};