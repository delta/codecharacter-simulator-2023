#include "game/pvp_game/pvp_game.hpp"
#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "logger/logger.hpp"
#include "utils/position.hpp"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <optional>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

PvPGame::PvPGame(std::vector<Attacker> player1_attackers,
                 std::vector<Attacker> player2_attackers,
                 unsigned player1_coins, unsigned player2_coins)
    : _player1_attackers(std::move(player1_attackers)),
      _player2_attackers(std::move(player2_attackers)),
      _player1_coins(player1_coins), _player2_coins(player2_coins) {
  ranges::for_each(
      this->_player1_attackers,
      [&, index = 0](const Attacker &player1_attacker) mutable {
        this->player1_attacker_id_to_index[player1_attacker.get_id()] = index++;
      });
  ranges::for_each(
      this->_player2_attackers,
      [&, index = 0](const Attacker &player2_attacker) mutable {
        this->player2_attacker_id_to_index[player2_attacker.get_id()] = index++;
      });
}

PvPGame PvPGame::simulate(
    const std::unordered_map<player1_attacker_id, player2_attacker_id>
        &player1_set_targets,
    const std::unordered_map<player2_attacker_id, player1_attacker_id>
        &player2_set_targets,
    const std::vector<std::pair<Position, AttackerType>>
        &spawn_positions_player1,
    const std::vector<std::pair<Position, AttackerType>>
        &spawn_positions_player2) const {

  const auto &prev_state_player1_attackers = this->get_player1_attackers();
  const auto &prev_state_player2_attackers = this->get_player2_attackers();

  std::vector<Attacker> player1_attackers(prev_state_player1_attackers.begin(),
                                          prev_state_player1_attackers.end());
  std::vector<Attacker> player2_attackers(prev_state_player2_attackers.begin(),
                                          prev_state_player2_attackers.end());
  ranges::for_each(player1_attackers, [](Attacker &player1_attacker) {
    player1_attacker.clear_destination();
  });

  ranges::for_each(player2_attackers, [](Attacker &player2_attacker) {
    player2_attacker.clear_destination();
  });

  ranges::for_each(
      player1_set_targets,
      [&](const std::pair<player1_attacker_id, player2_attacker_id> &entry) {
        auto player1_attacker_index =
            this->get_player1_attacker_index_by_id(entry.first);
        auto player2_attacker_index =
            this->get_player2_attacker_index_by_id(entry.second);
        if (player1_attacker_index.has_value() &&
            player2_attacker_index.has_value()) {
          player1_attackers[*player1_attacker_index].set_target(
              player2_attackers[*player2_attacker_index].get_id());
        }
      });

  ranges::for_each(
      player2_set_targets,
      [&](const std::pair<player2_attacker_id, player1_attacker_id> &entry) {
        auto player2_attacker_index =
            this->get_player2_attacker_index_by_id(entry.first);
        auto player1_attacker_index =
            this->get_player1_attacker_index_by_id(entry.second);
        if (player2_attacker_index.has_value() &&
            player1_attacker_index.has_value()) {
          player2_attackers[*player2_attacker_index].set_target(
              player1_attackers[*player1_attacker_index].get_id());
        }
      });

  ranges::for_each(player1_attackers, [&](Attacker &player1_attacker) mutable {
    std::optional<index_t> player2_attacker_index{std::nullopt};
    if (player1_attacker.is_target_set_by_player()) {
      player2_attacker_index = this->get_player2_attacker_index_by_id(
          player1_attacker.get_target_id());
    } else {
      player1_attacker.clear_target();
      player2_attacker_index =
          player1_attacker.get_nearest_attacker_index_for_pvp(
              prev_state_player1_attackers);
    }
    if (player2_attacker_index.has_value()) {
      if (player1_attacker.is_in_range(
              player2_attackers[*player2_attacker_index])) {
        player1_attacker.attack(player2_attackers[*player2_attacker_index],
                                'A');
      } else {
        player1_attacker.set_destination(
            player2_attackers[*player2_attacker_index].get_position());
      }
    }
  });

  ranges::for_each(player2_attackers, [&](Attacker &player2_attacker) mutable {
    std::optional<index_t> player1_attacker_index{std::nullopt};
    if (player2_attacker.is_target_set_by_player()) {
      player1_attacker_index = this->get_player1_attacker_index_by_id(
          player2_attacker.get_target_id());
    } else {
      player2_attacker.clear_target();
      player1_attacker_index =
          player2_attacker.get_nearest_attacker_index_for_pvp(
              prev_state_player2_attackers);
    }
    if (player1_attacker_index.has_value()) {
      if (player2_attacker.is_in_range(
              player2_attackers[*player1_attacker_index])) {
        player2_attacker.attack(player2_attackers[*player1_attacker_index],
                                'A');
      } else {
        player2_attacker.set_destination(
            player2_attackers[*player1_attacker_index].get_position());
      }
    }
  });

  ranges::for_each(player1_attackers, [](Attacker &player1_attacker) {
    player1_attacker.update_state();
  });
  ranges::for_each(player1_attackers, [](Attacker &player1_attacker) {
    player1_attacker.update_state();
  });

  ranges::for_each(player1_attackers, [&](Attacker &player1_attacker) {
    if (player1_attacker.is_target_set_by_player()) {
      player1_attacker.clear_target();
    }
  });

  ranges::for_each(player2_attackers, [&](Attacker &player2_attacker) {
    if (player2_attacker.is_target_set_by_player()) {
      player2_attacker.clear_target();
    }
  });

  player1_attackers.erase(
      remove_if(player1_attackers.begin(), player1_attackers.end(),
                [](const Attacker &player1_attacker) {
                  return player1_attacker.get_state() == Attacker::State::DEAD;
                }),
      player1_attackers.end());

  player2_attackers.erase(
      remove_if(player2_attackers.begin(), player2_attackers.end(),
                [](const Attacker &player2_attacker) {
                  return player2_attacker.get_state() == Attacker::State::DEAD;
                }),
      player2_attackers.end());

  auto player1_coins_left = this->get_player1_coins();
  auto positions = std::set<Position>{};
  ranges::for_each(spawn_positions_player1, [&](const auto &spawn_details) {
    const auto &[position, attacker_type] = spawn_details;
    const unsigned price = Attacker::attribute_dictionary[attacker_type].price;
    if (price > player1_coins_left) {
      return;
    }
    player1_coins_left -= price;

    if (Position::is_valid_pvp_spawn_position(
            position.get_x(), position.get_y(), PvpPlayerType::PLAYER1) &&
        !positions.contains(position)) {
      positions.insert(position);
      player1_attackers.push_back(Attacker::construct(attacker_type, position));
    }
  });

  auto player2_coins_left = this->get_player2_coins();
  ranges::for_each(spawn_positions_player2, [&](const auto &spawn_details) {
    const auto &[position, attacker_type] = spawn_details;
    const unsigned price = Attacker::attribute_dictionary[attacker_type].price;
    if (price > player2_coins_left) {
      return;
    }
    player2_coins_left -= price;

    if (Position::is_valid_pvp_spawn_position(
            position.get_x(), position.get_y(), PvpPlayerType::PLAYER2) &&
        !positions.contains(position)) {
      positions.insert(position);
      player2_attackers.push_back(Attacker::construct(attacker_type, position));
    }
  });

  return {std::move(player1_attackers), std::move(player2_attackers),
          player1_coins_left, player2_coins_left};
}

const std::vector<Attacker> &PvPGame::get_player1_attackers() const {
  return this->_player1_attackers;
}

const std::vector<Attacker> &PvPGame::get_player2_attackers() const {
  return this->_player2_attackers;
}

unsigned PvPGame::get_player1_coins() const { return this->_player1_coins; }

unsigned PvPGame::get_player2_coins() const { return this->_player2_coins; }

std::optional<PvPGame::index_t>
PvPGame::get_player1_attacker_index_by_id(player1_attacker_id id) const {
  if (this->player1_attacker_id_to_index.contains(id)) {
    return this->player1_attacker_id_to_index.at(id);
  }
  return std::nullopt;
}
std::optional<PvPGame::index_t>
PvPGame::get_player2_attacker_index_by_id(player2_attacker_id id) const {
  if (this->player2_attacker_id_to_index.contains(id)) {
    return this->player2_attacker_id_to_index.at(id);
  }
  return std::nullopt;
}
