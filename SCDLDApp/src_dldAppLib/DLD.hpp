/* Copyright 2022 Surface Concept GmbH */

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "glue.hpp"
#include "WorkerThread.hpp"
#include "PipeRatemeter.hpp"
#include "PipeImageXY.hpp"
#include "PipeTimeHisto.hpp"
#include "TimeBin.hpp"
#include "iDisconnectListener.hpp"
#include "HDF5Stream.hpp"

class DLD : public Glue<DLD>
{
  struct Data {
    std::string configfile;
    std::string statusmessage;
    int initialized;
    double exposure;
    double acquire_period;
    int acquire;
    int image_mode;
    int num_images;
    int image_counter;
    int detector_state;
    int ratemeter_max;
    int sizeT;
    double minTSI;
    double sizeTSI;
    Data();
  } data_;
public:
  DLD();
  int write_Initialize(int);
  int read_Initialize(int*);
  int write_ConfigFile(const std::string&);
  int read_ConfigFile(std::string&);
  int read_StatusMessage(std::string&);
  int read_DetectorState(int*);
  int write_Exposure(double);
  int read_Exposure(double*);
  int write_AcquirePeriod(double);
  int read_AcquirePeriod(double*);
  int write_Acquire(int);
  int read_Acquire(int*);
  int write_ImageMode(int);
  int read_ImageMode(int*);
  int write_NumImages(int);
  int read_NumImages(int*);
  int read_NumImagesCounter(int*);
  int write_BinX(int);
  int read_BinX(int*);
  int write_BinY(int);
  int read_BinY(int*);
  int write_MinX(int);
  int read_MinX(int*);
  int write_MinY(int);
  int read_MinY(int*);
  int write_SizeX(int);
  int read_SizeX(int*);
  int write_SizeY(int);
  int read_SizeY(int*);
  int write_SizeT(int);
  int read_SizeT(int*);
  int write_MinTSI(double);
  int read_MinTSI(double*);
  int write_SizeTSI(double);
  int read_SizeTSI(double*);
  int read_RatemeterMax(int*);
  int write_H5EventsFilePath(const std::string&);
  int read_H5EventsFilePath(std::string&);
  int write_H5EventsComment(const std::string&);
  int read_H5EventsComment(std::string&);
  int write_H5EventsActive(int);
  int read_H5EventsActive(int*);
  int read_H5EventsFileError(int*);

private:
  // functions
  int init_impl();
  void configure_pipes();
  void configure_pipes_liveimagexy();
  void configure_pipes_ratemeter();
  void configure_pipes_timehisto();
  void configure_timebin();
  void configure_hdf5stream();
  void cb_measurement_complete(int reason);
  static void cb_static_measurement_complete(void* priv, int reason);
  int start_measurement();
  // variables
  int dev_desc_;
  bool user_stop_request_ = false;
  std::chrono::steady_clock::time_point last_acq_start_;
  WorkerThread worker_;
  TimeBin timebin_; // keep this above timehisto_
  PipeRatemeter ratemeter_;
  PipeImageXY liveimagexy_;
  PipeTimeHisto timehisto_;
  HDF5Stream hdf5stream_;
  std::vector<iCreatedAtInit*> created_at_init_;
  std::vector<iEndOfMeasListener*> eom_listeners_;
  std::vector<iStartOfMeasListener*> som_listeners_;
  std::vector<iDisconnectListener*> disconnect_listeners_;
};
