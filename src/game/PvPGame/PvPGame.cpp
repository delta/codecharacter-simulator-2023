#include "game/PvPGame/PvPGame.hpp"
#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "logger/logger.hpp"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <optional>
#include <set>
#include <unordered_set>
#include <utility>

using namespace std;

PvPGame::PvPGame(std::vector<Attacker> player1_attackers,
                 std::vector<Attacker> player2_attackers, unsigned coins)
    : _player1_attackers(std::move(player1_attackers)),
      _player2_attackers(std::move(player2_attackers)), _coins(coins) {
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
        &player_set_targets,
    const std::vector<std::pair<Position, AttackerType>> &spawn_positions)
    const {

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
      player_set_targets,
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
                                'A'); // produces log
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

  auto coins_left = this->get_coins();
  auto positions = std::set<Position>{};
  ranges::for_each(spawn_positions, [&](const auto &spawn_details) {
    const auto &[position, attacker_type] = spawn_details;
    const unsigned price = Attacker::attribute_dictionary[attacker_type].price;
    if (price > coins_left) {
      return;
    }
    coins_left -= price;

    if (Position::is_valid_spawn_position(position.get_x(), position.get_y()) &&
        !positions.contains(position)) {
      positions.insert(position);
      player1_attackers.push_back(Attacker::construct(attacker_type, position));
    }
  });

  return {std::move(player1_attackers), std::move(player2_attackers),
          coins_left};
}

const std::vector<Attacker> &PvPGame::get_player1_attackers() const {
  return this->_player1_attackers;
}

const std::vector<Attacker> &PvPGame::get_player2_attackers() const {
  return this->_player2_attackers;
}

unsigned PvPGame::get_coins() const { return this->_coins; }

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
