#include "various.hpp"

#include <fstream>
#include <iostream>
#include <memory>


std::string
utils::slurp_file(std::string const& path)
{
  std::ifstream file = std::ifstream(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open \"" << path << "\"" << std::endl;
    return std::string("");
  }

  file.seekg(0, std::ios::end);
  auto const size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::unique_ptr<char[]> content = std::make_unique<char[]>(static_cast<size_t>(size) + 1ll);
  file.read(content.get(), size);
  content[static_cast<size_t>(size)] = '\0';

  return std::string(content.get());
}
