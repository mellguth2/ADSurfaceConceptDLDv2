/* Copyright 2022 Surface Concept GmbH */

#pragma once

#include <string>
#include <vector>
#include "glue.hpp"
#include "WorkerThread.hpp"
#include "PipeRatemeter.hpp"
#include "PipeImageXY.hpp"

class DLD : public Glue<DLD>
{
  struct Data {
    std::string configfile;
    std::string statusmessage;
    int initialized;
    double exposure;
    int acquire;
    int image_mode;
    int num_images;
    int data_type;
    Data();
  } data_;
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
  // functions
  int init_impl();
  void configure_pipes();
  void configure_pipes_liveimagexy();
  void configure_pipes_ratemeter();
  void cb_measurement_complete(int reason);
  static void cb_static_measurement_complete(void* priv, int reason);
  // variables
  int dev_desc_;
  WorkerThread worker_;
  PipeRatemeter ratemeter_;
  PipeImageXY liveimagexy_;
  std::vector<iCreatedAtInit*> created_at_init_;
  std::vector<iEndOfMeasListener*> eom_listeners_;
  std::vector<iStartOfMeasListener*> som_listeners_;
};
