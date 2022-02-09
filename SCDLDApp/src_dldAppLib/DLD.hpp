/* Copyright 2022 Surface Concept GmbH */

#pragma once

#include <string>
#include "glue.hpp"
#include "WorkerThread.hpp"

class DLD : public Glue<DLD>
{
  std::string configfile_;
  std::string statusmessage_;
  int initialized_;
  int dev_desc_;
  double exposure_;
  int acquire_;
public:
  DLD();
  int write_Initialize(int);
  int read_Initialize(int*);
  int write_ConfigFile(const std::string&);
  int read_ConfigFile(std::string&);
  int read_StatusMessage(std::string&);
  int write_Exposure(double);
  int read_Exposure(double*);
  int write_Acquire(int);
  int read_Acquire(int*);

private:
  int init_impl();
  void cb_measurement_complete(int reason);
  static void cb_static_measurement_complete(void* priv, int reason);
  WorkerThread worker_;
};
