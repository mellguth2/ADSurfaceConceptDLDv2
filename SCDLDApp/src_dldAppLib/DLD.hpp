/* Copyright 2022 Surface Concept GmbH */

#pragma once

#include <string>
#include <vector>
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
  int image_mode_;
  int num_images_;
  int data_type_;
  std::vector<unsigned> ratemeter_;
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
  int write_ImageMode(int);
  int read_ImageMode(int*);
  int write_NumImages(int);
  int read_NumImages(int*);
  int write_DataType(int);
  int read_DataType(int*);


private:
  int init_impl();
  void cb_measurement_complete(int reason);
  static void cb_static_measurement_complete(void* priv, int reason);
  WorkerThread worker_;
};
