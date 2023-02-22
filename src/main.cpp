#include "defender/defender.hpp"
#include "game/game.hpp"
#include "game/pvp_game/pvp_game.hpp"
#include "logger/logger.hpp"
#include "utils/argument_reader.hpp"
#include "utils/file_descriptor.hpp"
#include "utils/game_map.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

enum GameType { PvP, Normal };

GameType stringToGameType(std::string gameType) {
  std::for_each(gameType.begin(), gameType.end(),
                [](char &c) { c = std::tolower(c); });

  if (gameType == "normal") {
    return GameType::Normal;
  } else if (gameType == "pvp") {
    return GameType::PvP;
  } else {
    throw std::invalid_argument("Unable to convert string to GameType enum");
  }
}

void normalGame() {
  std::cout.setf(std::ios::unitbuf);
  unsigned turns, coins;
  std::cin >> turns >> coins;

  unsigned n_attacker_types;
  std::cin >> n_attacker_types;
  for (unsigned attacker_type_id = 1; attacker_type_id <= n_attacker_types;
       ++attacker_type_id) {
    unsigned hp, range, attack_power, speed, price;
    bool is_aerial;
    std::cin >> hp >> range >> attack_power >> speed >> price >> is_aerial;
    Attacker::attribute_dictionary.insert(std::make_pair(
        AttackerType(attacker_type_id),
        Attributes(hp, range, attack_power, speed, price, is_aerial)));
  }

  unsigned n_defender_types;
  std::cin >> n_defender_types;
  for (unsigned defender_type_id = 1; defender_type_id <= n_defender_types;
       ++defender_type_id) {
    unsigned hp, range, attack_power, speed, price;
    bool is_aerial;
    std::cin >> hp >> range >> attack_power >> speed >> price >> is_aerial;
    Defender::attribute_dictionary.insert(std::make_pair(
        DefenderType(defender_type_id),
        Attributes(hp, range, attack_power, speed, price, is_aerial)));
  }

  auto map = Map::get(std::cin);
  auto defenders = map.spawn_defenders();

  auto initial_hp = std::accumulate(defenders.begin(), defenders.end(), 0,
                                    [](unsigned acc, const Defender &defender) {
                                      return acc + defender.get_hp();
                                    });

  Logger::log_init(defenders);

  Game game({}, defenders, coins);

  for (size_t turn = 0; turn < turns; ++turn) {
    Logger::log_turn(turn);

    unsigned n_attackers;
    std::cin >> n_attackers;

    auto spawn_positions = std::vector<std::pair<Position, AttackerType>>();
    while (n_attackers-- > 0) {
      unsigned type_id, x, y;
      std::cin >> type_id >> x >> y;
      spawn_positions.emplace_back(
          std::make_pair(Position(x, y), AttackerType(type_id)));
    }

    //  Get all the manually to be set targets as input, we need attacker's id
    //  and targetted defender id
    std::unordered_map<Game::attacker_id, Game::defender_id> player_set_targets;
    int no_of_player_set_targets = 0;
    std::cin >> no_of_player_set_targets;
    while ((no_of_player_set_targets--) > 0) {
      Game::attacker_id att_id = 0;
      Game::defender_id def_id = 0;
      std::cin >> att_id >> def_id;
      player_set_targets[att_id] = def_id;
    }

    game = game.simulate(player_set_targets, spawn_positions);

    auto active_attackers = game.get_attackers();
    std::cout << active_attackers.size() << "\n";
    std::ranges::for_each(active_attackers, [](const Attacker &attacker) {
      std::cout << attacker.get_id() << " " << attacker.get_position().get_x()
                << " " << attacker.get_position().get_y() << " "
                << (int)attacker.get_type() << " " << attacker.get_hp() << "\n";
    });

    auto active_defenders = game.get_defenders();
    std::cout << active_defenders.size() << "\n";
    std::ranges::for_each(active_defenders, [](const Defender &defender) {
      std::cout << defender.get_id() << " " << defender.get_position().get_x()
                << " " << defender.get_position().get_y() << " "
                << (int)defender.get_type() << " " << defender.get_hp() << "\n";
    });

    std::cout << game.get_coins() << "\n";

    auto current_hp =
        std::accumulate(active_defenders.begin(), active_defenders.end(), 0,
                        [](unsigned acc, const Defender &defender) {
                          return acc + defender.get_hp();
                        });

    Logger::log_destruction(
        initial_hp == 0 ? 0 : (initial_hp - current_hp) * 100.0 / initial_hp);
    Logger::log_coins(game.get_coins());
  }
  Logger::log_end();

  // TODO: Figure out a way to save and extract logs from driver
  std::cerr << Logger::get_log();
}

void pvpGame(int p1_in, int p1_out, int p2_in, int p2_out, std::ofstream &fs) {
  // TODO: Implement file-descriptor to input/output stream
  // and actual gameplay to generate logs
  FileDescriptorInput player1_input(p1_in, p1_out);
  FileDescriptorInput player2_input(p2_in, p2_out);

  std::cout.setf(std::ios::unitbuf);
  fs << "Getting initial parameters\n";
  unsigned turns, player1_coins, player2_coins, coins;
  std::cin >> turns >> coins;

  fs << "Turns: " << turns << ", Coins: " << coins << "\n";

  player1_coins = coins;
  player2_coins = coins;

  unsigned n_attacker_types;
  std::cin >> n_attacker_types;
  fs << "N_Attackers_types: " << n_attacker_types << "\n";

  for (unsigned attacker_type_id = 1; attacker_type_id <= n_attacker_types;
       ++attacker_type_id) {
    unsigned hp, range, attack_power, speed, price;
    bool is_aerial;
    std::cin >> hp >> range >> attack_power >> speed >> price >> is_aerial;
    fs << "Attacker > hp: " << hp << ", range: " << range
       << ", power: " << attack_power << ", speed: " << speed
       << ", price: " << price << ", is_aerial: " << is_aerial << "\n";
    Attacker::attribute_dictionary.insert(std::make_pair(
        AttackerType(attacker_type_id),
        Attributes(hp, range, attack_power, speed, price, is_aerial)));
  }

  unsigned n_defender_types;
  std::cin >> n_defender_types;
  for (unsigned defender_type_id = 1; defender_type_id <= n_defender_types;
       ++defender_type_id) {
    unsigned hp, range, attack_power, speed, price;
    bool is_aerial;
    std::cin >> hp >> range >> attack_power >> speed >> price >> is_aerial;
    Defender::attribute_dictionary.insert(std::make_pair(
        DefenderType(defender_type_id),
        Attributes(hp, range, attack_power, speed, price, is_aerial)));
  }

  // auto map = Map::get(std::cin);
  // auto defenders = map.spawn_defenders();

  // auto initial_hp = std::accumulate(defenders.begin(), defenders.end(), 0,
  //                                   [](unsigned acc, const Defender
  //                                   *defender) {
  //                                     return acc + defender->get_hp();
  //                                   });

  // Logger::log_init(defenders);

  fs << "Reading player1 stream\n";

  PvPGame game({}, {}, player1_coins, player2_coins);
  fs << "Initial parameters done!\n";

  // fs.close();
  for (size_t turn = 0; turn < turns; ++turn) {
    fs << "Turn: " << turn << "\n";
    fs.flush();
    Logger::log_turn(turn);

    unsigned n_player1_attackers;
    player1_input.get() >> n_player1_attackers;
    fs << "player1 attackers count: " << n_player1_attackers << "\n";

    auto player1_spawn_positions =
        std::vector<std::pair<Position, AttackerType>>();
    while (n_player1_attackers-- > 0) {
      fs << Logger::get_log() << "\n";
      unsigned type_id, x, y;
      player1_input.get() >> type_id >> x >> y;
      fs << "id: " << type_id << ", x: " << x << ", y: " << y << "\n";
      fs.flush();
      player1_spawn_positions.emplace_back(
          std::make_pair(Position(x, y), AttackerType(type_id)));
    }

    unsigned n_player2_attackers;
    player2_input.get() >> n_player2_attackers;
    fs << "player2 attackers count: " << n_player2_attackers << "\n";
    fs.flush();

    auto player2_spawn_positions =
        std::vector<std::pair<Position, AttackerType>>();
    while (n_player2_attackers-- > 0) {
      unsigned type_id, x, y;
      player2_input.get() >> type_id >> x >> y;
      fs << "id: " << type_id << ", x: " << x << ", y: " << y << "\n";
      fs.flush();
      player2_spawn_positions.emplace_back(
          std::make_pair(Position(x, y), AttackerType(type_id)));
    }

    //  Get all the manually to be set targets as input, we need attacker's id
    //  and targetted defender id
    std::unordered_map<PvPGame::player1_attacker_id,
                       PvPGame::player2_attacker_id>
        player1_set_targets;
    int no_of_player1_set_targets = 0;
    player1_input.get() >> no_of_player1_set_targets;
    fs << "Player1 set targets: " << no_of_player1_set_targets << "\n";
    fs.flush();
    while ((no_of_player1_set_targets--) > 0) {
      PvPGame::player1_attacker_id player1_att_id = 0;
      PvPGame::player2_attacker_id player2_att_id = 0;
      player1_input.get() >> player1_att_id >> player2_att_id;
      player1_set_targets[player1_att_id] = player2_att_id;
    }

    std::unordered_map<PvPGame::player1_attacker_id,
                       PvPGame::player2_attacker_id>
        player2_set_targets;
    int no_of_player2_set_targets = 0;
    player2_input.get() >> no_of_player2_set_targets;
    fs << "Player2 set targets: " << no_of_player2_set_targets << "\n";
    fs.flush();
    while ((no_of_player2_set_targets--) > 0) {
      PvPGame::player1_attacker_id player1_att_id = 0;
      PvPGame::player2_attacker_id player2_att_id = 0;
      player2_input.get() >> player1_att_id >> player2_att_id;
      player2_set_targets[player2_att_id] = player1_att_id;
    }

    game = game.simulate(player1_set_targets, player2_set_targets,
                         player1_spawn_positions,
                         player2_spawn_positions); // pass player2_set_targets
                                                   // also as a parameter

    fs << "printing player1 active attackers\n";
    fs.flush();

    auto player1_active_attackers = game.get_player1_attackers();
    player1_input.put() << player1_active_attackers.size() << "\n";
    std::ranges::for_each(
        player1_active_attackers, [&player1_input](const Attacker &attacker) {
          player1_input.put()
              << attacker.get_id() << " " << attacker.get_position().get_x()
              << " " << attacker.get_position().get_y() << " "
              << (int)attacker.get_type() << " " << attacker.get_hp() << "\n";
        });

    player1_input.put().flush();

    fs << "priting player1 active attackers done\n";
    fs.flush();

    fs << "printing player2 active attackers\n";
    fs.flush();

    auto player2_active_attackers = game.get_player2_attackers();
    player2_input.put() << player2_active_attackers.size() << "\n";
    std::ranges::for_each(
        player2_active_attackers, [&player2_input](const Attacker &attacker) {
          player2_input.put()
              << attacker.get_id() << " " << attacker.get_position().get_x()
              << " " << attacker.get_position().get_y() << " "
              << (int)attacker.get_type() << " " << attacker.get_hp() << "\n";
        });

    player2_input.put().flush();

    fs << "priting player2 active attackers done\n";
    fs.flush();

    player1_input.put() << game.get_player1_coins() << "\n";
    player2_input.put() << game.get_player2_coins()
                        << "\n"; // separate coins for player1 and player2

    player1_input.put().flush();
    player2_input.put().flush();

    // auto current_hp =
    //     std::accumulate(active_defenders.begin(), active_defenders.end(), 0,
    //                     [](unsigned acc, const Defender *defender) {
    //                       return acc + defender->get_hp();
    //                     });

    // Logger::log_destruction(
    //     initial_hp == 0 ? 0 : (initial_hp - current_hp) * 100.0 /
    //     initial_hp);

    Logger::log_coins(game.get_player1_coins());
    Logger::log_coins(
        game.get_player2_coins()); // separate coins for player1 and player 2
  }
  Logger::log_end();

  // TODO: Figure out a way to save and extract logs from driver
  std::cerr << Logger::get_log() << std::endl;
}

int main(int argc, char *argv[]) {
  std::ofstream fs;
  fs.open("log.txt");
  fs << "ok";

  ArgumentReader commandLineReader = ArgumentReader(argc, argv);
  auto gameType = stringToGameType(commandLineReader.get("--type", "normal"));

  switch (gameType) {
  case GameType::Normal:
    normalGame();
    break;

  case GameType::PvP:
    fs << "PVP\n";
    int p1_in = std::stoi(commandLineReader.get("p1_in"));
    int p2_in = std::stoi(commandLineReader.get("p2_in"));
    int p1_out = std::stoi(commandLineReader.get("p1_out"));
    int p2_out = std::stoi(commandLineReader.get("p2_out"));
    // two istream from p1in p2in
    // ..
    fs << p1_in << ", " << p2_in << ", " << p1_out << ", " << p2_out << "\n";

    pvpGame(p1_in, p1_out, p2_in, p2_out, fs);
    fs << "final bro";
    // fs.close();
    break;
  }
}
