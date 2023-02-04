#include "attacker/attacker.hpp"
#include "defender/defender.hpp"
#include "mines/mines.hpp"

#include <catch2/catch.hpp>

SCENARIO("Mines::get_nearest_defender_index") {
  GIVEN("a list of attackers of varying distances from the mine") {
    Mines::attribute_dictionary.clear();
    Mines::attribute_dictionary.insert(
        std::make_pair(MineTypes::M1, Attributes(0, 2, 0, 0, 0)));
    Mines mine = Mines::construct(MineTypes::M1, {0, 0});

    WHEN("the given list is empty") {
      std::vector<Defender> defenders;

      auto nearest_defender_index =
          mine.get_nearest_defender_indexes(defenders);

      THEN("nearest defender does not exist") {
        REQUIRE(nearest_defender_index.has_value() == false);
      }
    }

    WHEN("list of defenders") {
      std::vector<Defender> defenders{
          Defender::construct(DefenderType::D1, {5, 0}),
          Defender::construct(DefenderType::D1, {3, 0}),
          Defender::construct(DefenderType::D1, {1, 0}),
          Defender::construct(DefenderType::D1, {7, 0}),
      };

      auto nearest_defender_index =
          mine.get_nearest_defender_indexes(defenders);

      THEN("nearest defender is the closest defender") {
        REQUIRE(nearest_defender_index.has_value() == true);
        REQUIRE(nearest_defender_index.value().size() == 2);
        REQUIRE(nearest_defender_index.value().at(Mines::RANGE_1).size() == 1);
        REQUIRE(nearest_defender_index.value()
                    .at(Mines::RANGE_1)
                    .at(0)
                    .get_position()
                    .get_x() == 1);
      }
    }
  }
}