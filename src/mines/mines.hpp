#include "actor/actor.hpp"
#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "utils/attributes.hpp"
#include "utils/position.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

enum MineTypes { M1 = 1, M2, M3, M4, M5 };
class Attacker;
class Defender;
class Mines : public Actor {
public:
  static inline std::unordered_map<MineTypes, Attributes> attribute_dictionary;

  enum State { DORMANT, BLASTING, BLASTED };
  enum Range {
    RANGE_0 = 0,
    RANGE_1 = 1,
    RANGE_2 = 2,
    RANGE_3 = 3,
    RANGE_4 = 4,
    RANGE_5 = 5
  };
  std::vector<Range> mine_ranges = {RANGE_0, RANGE_1, RANGE_2,
                                    RANGE_3, RANGE_4, RANGE_5};

  Mines(MineTypes mineType, Position position, unsigned hp, unsigned range,
        unsigned attack_power, unsigned price, State state = State::DORMANT)
      : Actor{_id_counter++, position, hp, range, attack_power, price},
        _type(mineType), _state(state) {}

  [[nodiscard]] static Mines construct(MineTypes mineType, Position p);

  void
  attack_mines(std::unordered_map<Range, std::vector<Attacker>> &attackers,
               std::unordered_map<Range, std::vector<Defender>> &defenders);

  void
  attack_mines(std::unordered_map<Range, std::vector<Attacker>> &attackers);

  [[nodiscard]] std::optional<std::unordered_map<Range, std::vector<Attacker>>>
  get_nearest_attacker_indexes(std::vector<Attacker> &attackers) const;

  [[nodiscard]] std::optional<std::unordered_map<Range, std::vector<Defender>>>
  get_nearest_defender_indexes(std::vector<Defender> &defenders) const;

  [[nodiscard]] bool is_in_attack_position(
      std::unordered_map<Mines::Range, std::vector<Attacker>> &attacker) const;

  void set_state(State _state);

  [[nodiscard]] State get_state() const;

  [[nodiscard]] MineTypes get_type() const;

  void update_state() final;

private:
  static inline size_t _id_counter = 0;
  MineTypes _type;
  State _state;
};
