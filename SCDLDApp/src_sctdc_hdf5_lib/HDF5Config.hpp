#ifndef GUI2_HDF5CONFIG_HPP
#define GUI2_HDF5CONFIG_HPP

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/


#include <string>

struct EventDataFieldSelection
{
  typedef unsigned type;

  static const type STARTCTR = 0x1;
  static const type TIMETAG = 0x2;
  static const type SUBDEV = 0x4;
  static const type CHANNEL = 0x8;
  static const type SUM = 0x10;
  static const type DIF1 = 0x20;
  static const type DIF2 = 0x40;
  static const type MRC = 0x80; // master reset counter
  static const type ADC = 0x100;
  static const type SIGBIT = 0x200;
  static const type TIMEDATA = 0x400; // (tdc event only, not implemented)
  static const type ALL_DLD = 0x3FF;

  type value; // bitmask of the above constants

  EventDataFieldSelection() : value(0) {}
};

struct HDF5Config {
  std::string base_path;
  std::string user_comment;
  EventDataFieldSelection datasel;
  bool overwrite; // (without effect)
};

#endif
