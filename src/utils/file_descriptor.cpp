#include "utils/file_descriptor.hpp"
#include <cerrno>
#include <fstream>
#include <string>
using namespace std;

FileDescriptorInput::FileDescriptorInput(int input_fd, int output_fd)
    : _input_fd(input_fd), _output_fd(output_fd) {
  if (_input_fd == -1 || _output_fd == -1) {
    std::cerr << "Error occurred while opening an fd";
  }
  std::string input_fd_file = "/proc/self/fd" + std::to_string(_input_fd);
  std::string output_fd_file = "/proc/self/fd" + std::to_string(_output_fd);

  std::ifstream &istream_file = this->get();
  std::ofstream &ostream_file = this->put();
  istream_file.open(input_fd_file);
  ostream_file.open(output_fd_file);

  if (!istream_file) {
    std::cerr << "Failed in opening the input file with error no - " << errno
              << endl;
    istream_file.close();
  }
  if (!ostream_file) {
    std::cerr << "Failed in opening the output file with error no - " << errno
              << endl;
    ostream_file.close();
  }
}

FileDescriptorInput::~FileDescriptorInput() {
  this->get().close();
  this->put().close();
}
std::ifstream &FileDescriptorInput ::get() { return this->_ifs; }

std::ofstream &FileDescriptorInput ::put() { return this->_ofs; }