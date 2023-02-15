#pragma once

#include "actor/actor.hpp"
#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "utils/position.hpp"

#include <memory>
#include <vector>

class PvPGame {

public:
  PvPGame(std::vector<Attacker> player1_attackers,
          std::vector<Attacker> player2_attackers, unsigned coins);

  using player2_attacker_id = size_t;
  using player1_attacker_id = size_t;
  using index_t = size_t;

  [[nodiscard]] const std::vector<Attacker> &get_player1_attackers() const;

  [[nodiscard]] const std::vector<Attacker> &get_player2_attackers() const;

  [[nodiscard]] PvPGame
  simulate(const std::unordered_map<player1_attacker_id, player2_attacker_id>
               &player_set_targets,
           const std::vector<std::pair<Position, AttackerType>>
               &spawn_positions) const;

  [[nodiscard]] unsigned get_coins() const;

private:
  std::optional<index_t>
  get_player1_attacker_index_by_id(player1_attacker_id id) const;
  std::optional<index_t>
  get_player2_attacker_index_by_id(player2_attacker_id id) const;

  std::unordered_map<player1_attacker_id, size_t> player1_attacker_id_to_index;
  std::unordered_map<player2_attacker_id, size_t> player2_attacker_id_to_index;
  std::vector<Attacker> _player1_attackers, _player2_attackers;
  unsigned _coins;
};
