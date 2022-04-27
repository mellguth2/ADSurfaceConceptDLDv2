/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#define LIB_VERSION "0.1.3"

#include <unordered_map>
#include <utility>
#include "HDF5Writer.hpp"
#include "HDF5Config.hpp"
#include <scTDC.h>
#include "scTDC_hdf5.h"
#include "scTDC_hdf5_error_codes.h"

namespace {
  struct HDF5Instance {
    HDF5Writer* writer;
    HDF5Config* cfg;
    HDF5Instance() {
      writer = new HDF5Writer;
      cfg = new HDF5Config;
    }
    ~HDF5Instance() {
      if (writer) delete writer;
      if (cfg) delete cfg;
    }
    HDF5Instance(HDF5Instance&& o) { // move constructor ok
      writer = o.writer;
      o.writer = nullptr;
      cfg = o.cfg;
      o.cfg = nullptr;
    }
    HDF5Instance& operator=(HDF5Instance&& o) { // move assignment
      if (writer) delete writer;
      if (cfg) delete cfg;
      writer = o.writer;
      if (o.writer) delete o.writer;
      cfg = o.cfg;
      if (o.cfg) delete o.cfg;
      return *this;
    }

    HDF5Instance(const HDF5Instance&) = delete; // forbid copy constructor
    HDF5Instance& operator=(const HDF5Instance&) = delete; // forbid copy assign
  };

  std::unordered_map<int, HDF5Instance> instances;

  std::string version_str(LIB_VERSION);
} // anonymous namespace

int sc_tdc_hdf5_create()
{
  try {
    std::size_t imax = instances.size();
    for (std::size_t i = imax; i < 0xFFFFFFFFFFFFFFFFull; i--) {
      if (instances.count(i)==0) {
        HDF5Instance h;
        instances.emplace(std::make_pair(i, std::move(h)));
        return i;
      }
    }
    return ERR_UNSPECIFIED; // this is practically impossible
  }
  catch (std::bad_alloc&) {
    return ERR_BAD_ALLOC;
  }
}

int sc_tdc_hdf5_destroy(int hdf5obj)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end())
    instances.erase(it);
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_connect(int hdf5obj, int dev_desc)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    it->second.writer->devConnected(dev_desc);
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_disconnect(int hdf5obj)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    it->second.writer->devDisconnected();
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_setactive(int hdf5obj, int active)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    int retval = (it->second.writer->setActive(active > 0)) ? 1 : 0;
    if (retval == active) {
      return retval;
    }
    if (active == 1 && retval == 0) {
      if (it->second.writer->fileError()) {
        return ERR_FILE;
      }
      return retval;
    }
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_isactive(int hdf5obj)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    return (it->second.writer->active()) ? 1 : 0;
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_cfg_outfile(int hdf5obj, const char *fpath)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    it->second.cfg->base_path = std::string(fpath);
    it->second.writer->setConfig(*(it->second.cfg));
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_cfg_comment(int hdf5obj, const char *msg)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    it->second.cfg->user_comment = std::string(msg);
    it->second.writer->setConfig(*(it->second.cfg));
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

int sc_tdc_hdf5_cfg_datasel(int hdf5obj, unsigned mask)
{
  auto it = instances.find(hdf5obj);
  if (it != instances.end()) {
    it->second.cfg->datasel.value = mask;
    it->second.writer->setConfig(*(it->second.cfg));
  }
  else
    return ERR_INSTANCE_NOTEXIST;
  return 0;
}

void sc_tdc_hdf5_version(char *buf, size_t len)
{
  if (buf==nullptr) return;
  size_t lenleft = len;
  for (std::size_t i = 0; i < version_str.size() && lenleft > 0; i++) {
    buf[i] = version_str[i];
    lenleft--;
  }
  // add zero termination of (char*) string
  if (len < version_str.size() + 1)
    buf[len-1] = '\0';
  else
    buf[version_str.size()] = '\0';
}
