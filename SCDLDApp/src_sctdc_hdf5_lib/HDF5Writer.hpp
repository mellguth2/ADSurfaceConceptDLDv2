#ifndef HDF5WRITER_H
#define HDF5WRITER_H

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#include <memory>
#include "HDF5Config.hpp"

class HDF5WriterImpl; // forward declaration used in unique_ptr requires that a
// destructor of HDF5Writer is defined in the cpp file (i.e. implicit destructor
// won't work)

class HDF5Writer
{
public:
  HDF5Writer();
  ~HDF5Writer();
  /**
   * @brief devConnected notify the HDF5Writer that your device is connected
   * @param devdesc
   */
  void devConnected(int devdesc);

  /**
   * @brief devDisconnected notify the HDF5Writer that your dev is disconnected
   */
  void devDisconnected();

  /**
   * @brief deviceDescriptor get cached device descriptor id
   * @return device descriptor id
   */
  int deviceDescriptor() const;

  /**
   * @brief setActive (de)activate HDF5 streaming (starts/stops worker thread)
   * @param active true to start streaming, false to terminate and close file
   */
  bool setActive(bool active);

  /**
   * @brief active the current activity status of HDF5 streaming
   * @return true, if active. false, if inactive
   */
  bool active() const;

  /**
   * @brief setConfig set config parameters
   */
  void setConfig(const HDF5Config&);

  /**
   * @brief config retrieve config parameters
   * @return object holding the config parameter values
   */
  const HDF5Config& config() const;

  /**
   * @brief query error while opening file
   * @return true if error occurred
   */
  bool fileError() const;

private:
  std::unique_ptr<HDF5WriterImpl> p;
};

#endif // HDF5WRITER_H
