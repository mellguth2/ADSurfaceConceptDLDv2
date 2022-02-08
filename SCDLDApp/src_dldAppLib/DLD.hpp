#pragma once

#include <string>
#include "glue.hpp"

class DLD : public Glue<DLD>
{
  std::string configfile_;
  int initialized_;
  int dev_desc_;
public:
  DLD();
  int write_Initialize(int);
  int write_ConfigFile(const std::string&);
  int read_ConfigFile(std::string&);
};
