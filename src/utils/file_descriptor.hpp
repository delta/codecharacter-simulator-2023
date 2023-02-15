#include <algorithm>
#include <bits/types/FILE.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

class PvPFDInput {
public:
  PvPFDInput(int input_fd, int output_fd);
  ~PvPFDInput();
  std::ifstream &get();
  std::ofstream &put();
  void close_stream();

private:
  int _input_fd, _output_fd;
  std::ifstream _ifs;
  std::ofstream _ofs;
};