#pragma once

#include <map>
#include <string>
#include <vector>

class ArgumentReader {
public:
  ArgumentReader(int, char **);
  [[nodiscard]] std::string get(std::string, std::string);
  [[nodiscard]] std::string get(std::string);

private:
  const char SepChar = '=';
  std::vector<std::string> _args;
  std::map<std::string, std::string> _dict;
  void process(void);
};
