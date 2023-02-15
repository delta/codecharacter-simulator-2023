#include "utils/argument_reader.hpp"

#include <catch2/catch.hpp>

SCENARIO("Single Key") {
  GIVEN("One key-value pair") {
    int argc = 1;
    char **argv = new char *[1];
    argv[0] = (char *)"--type=Normal";

    ArgumentReader args = ArgumentReader(argc, argv);

    THEN("Getting args") { REQUIRE(args.get("--type") == "Normal"); }
  }
}

SCENARIO("Key not present") {
  GIVEN("No args") {
    int argc = 0;
    char **argv = nullptr;

    ArgumentReader args = ArgumentReader(argc, argv);

    THEN("Getting args") { REQUIRE(args.get("--type") == ""); }
  }
}

SCENARIO("Single key present multiple time") {
  GIVEN("multiple pairs with same keys") {
    int argc = 2;
    char **argv = new char *[2];

    argv[0] = (char *)"--type=Normal";
    argv[1] = (char *)"--type=PvP";

    ArgumentReader args = ArgumentReader(argc, argv);

    THEN("Getting args") { REQUIRE(args.get("--type") == "PvP"); }
  }
}
