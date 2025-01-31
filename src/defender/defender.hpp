#pragma once

#include "actor/actor.hpp"
#include "utils/attributes.hpp"
#include "utils/position.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

enum class DefenderType { D1 = 1, D2, D3, D4, D5 };

class Attacker;

class Defender : public Actor {

public:
  static inline std::unordered_map<DefenderType, Attributes>
      attribute_dictionary;

  enum class State { IDLE, ATTACKING, DEAD };

  Defender(DefenderType type, Position position, unsigned hp, unsigned range,
           unsigned attack_power, unsigned price, bool is_aerial,
           State state = State::IDLE)
      : Actor{_id_counter++, position, hp,       range,
              attack_power,  price,    is_aerial},
        _type(type), _state(state) {}

  [[nodiscard]] static Defender construct(DefenderType type, Position p);

  void attack(Actor &opponent) const override;

  [[nodiscard]] std::optional<size_t>
  get_nearest_attacker_index(const std::vector<Attacker> &attackers) const;

  void update_state() final;

  void set_state(State s);

  [[nodiscard]] DefenderType get_type() const;

  [[nodiscard]] State get_state() const;

private:
  static inline size_t _id_counter = 0;
  DefenderType _type;
  State _state;
};
