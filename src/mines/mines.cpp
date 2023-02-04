#include "mines/mines.hpp"
#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "utils/attributes.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <ranges>
#include <vector>

Mines Mines::construct(MineTypes type, Position p) {
  Attributes attr = Mines::attribute_dictionary[type];
  return {type, p, attr.hp, attr.range, attr.attack_power, attr.price};
}

std::optional<std::unordered_map<Mines::Range, std::vector<Attacker>>>
Mines::get_nearest_attacker_indexes(std::vector<Attacker> &attackers) const {
  unsigned range = this->get_range();
  std::unordered_map<Range, std::vector<Attacker>> nearest_attackers;
  auto range0_conditon = [this](const Attacker &attacker) {
    return (int)attacker.get_position().distance_to(this->get_position()) <=
           RANGE_0;
  };
  auto range0_attackers = attackers | std::views::filter(range0_conditon);
  nearest_attackers[RANGE_0].assign(range0_attackers.begin(),
                                    range0_attackers.end());
  for (unsigned i = 1; i < range; i++) {
    Range current_range = mine_ranges[i];
    Range lower_limit_range = mine_ranges[i - 1];
    auto range_condition = [current_range, lower_limit_range,
                            this](const Attacker &attacker) {
      return (int)attacker.get_position().distance_to(this->get_position()) <=
                 current_range &&
             (int)attacker.get_position().distance_to(this->get_position()) >=
                 lower_limit_range;
    };
    auto range_attackers = attackers | std::views::filter(range_condition);
    nearest_attackers[current_range].assign(range_attackers.begin(),
                                            range_attackers.end());
  }
  return nearest_attackers;
}

std::optional<std::unordered_map<Mines::Range, std::vector<Defender>>>
Mines::get_nearest_defender_indexes(std::vector<Defender> &defender) const {
  unsigned range = this->get_range();
  if (defender.empty()) {
    return std::nullopt;
  }
  std::unordered_map<Range, std::vector<Defender>> nearest_defender;
  auto range0_conditon = [this](const Defender &defender) {
    return (int)defender.get_position().distance_to(this->get_position()) <=
           RANGE_0;
  };
  auto range0_defenders = defender | std::views::filter(range0_conditon);
  nearest_defender[RANGE_0].assign(range0_defenders.begin(),
                                   range0_defenders.end());
  for (unsigned i = 1; i < range; i++) {
    Range current_range = mine_ranges[i];
    Range lower_limit_range = mine_ranges[i - 1];
    auto range_condition = [current_range, lower_limit_range,
                            this](const Defender &defender) {
      return (int)defender.get_position().distance_to(this->get_position()) <=
                 current_range &&
             (int)defender.get_position().distance_to(this->get_position()) >=
                 lower_limit_range;
    };
    auto range_defenders = defender | std::views::filter(range_condition);
    nearest_defender[current_range].assign(range_defenders.begin(),
                                           range_defenders.end());
  }
  return nearest_defender;
}

void Mines::attack_mines(
    std::unordered_map<Mines::Range, std::vector<Attacker>> &attackers,
    std::unordered_map<Mines::Range, std::vector<Defender>> &defenders) {
  for (auto &range : mine_ranges) {
    for (auto &attacker : attackers[range]) {
      attacker.take_damage(this->get_attack_power() * range / 100);
    }
    for (auto &defender : defenders[range]) {
      defender.take_damage(this->get_attack_power() * range / 100);
    }
  }
}

void Mines::attack_mines(
    std::unordered_map<Mines::Range, std::vector<Attacker>> &attackers) {
  for (auto &range : mine_ranges) {
    for (auto &attacker : attackers[range]) {
      attacker.take_damage(this->get_attack_power() / range);
    }
  }
}

void Mines::update_state() {
  if (this->get_hp() <= 0) {
    this->_state = State::BLASTED;
  } else {
    this->_state = State::DORMANT;
  }
}

bool Mines ::is_in_attack_position(
    std::unordered_map<Mines::Range, std::vector<Attacker>> &attacker) const {
  return !attacker[RANGE_0].empty();
}

void Mines::set_state(State _state) { this->_state = _state; }

Mines::State Mines::get_state() const { return this->_state; }

MineTypes Mines::get_type() const { return this->_type; }
