#include "argument_reader.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

ArgumentReader::ArgumentReader(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    std::string cur(argv[i]);
    _args.push_back(cur);
  }
  ArgumentReader::process();
};

void ArgumentReader::process(void) {
  std::for_each(_args.begin(), _args.end(), [&](std::string &arg) {
    size_t found = arg.find(SepChar);

    if (found == std::string::npos) {
      return;
    }

    std::string key = arg.substr(0, found);
    std::string value = arg.substr(found + 1);
    _dict[key] = value;
  });
}

std::string ArgumentReader::get(std::string key) { return this->get(key, ""); }

std::string ArgumentReader::get(std::string key,
                                std::string default_value = "") {
  if (_dict.count(key) > 0) {
    return _dict[key];
  }

  return default_value;
}
