#ifndef HDF5DATAFILE_H
#define HDF5DATAFILE_H

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#include <string>
#include <vector>
#include <memory>
#include <H5Ipublic.h>

namespace H5 { class H5File; }
namespace H5 { class DataSet; }

//struct HDF5WriterImplThreadLocals
//{
//  std::vector<std::shared_ptr<H5::DataSet>> a;
//  std::shared_ptr<H5::H5File> fileID;
//};


class HDF5DataFile
{
  static const unsigned RANK = 1;
  static const unsigned EVENTS_PER_CHUNK = 50000;
  static const unsigned PACKLEVEL = 0;

  std::unique_ptr<H5::H5File> f_;
  std::vector<std::shared_ptr<H5::DataSet>> datasets_;

public:
  HDF5DataFile();
  ~HDF5DataFile();
  void open(const std::string& fpath);
  void close();
  bool isOpen() const;

  void closeDataSets();

  // addDataSet returns index of the newly created dataset in datasets_ vector
  template <typename T>
  std::size_t addDataSet(const char* name_arg);

  std::size_t addDataSetRaw(const char* name_arg, hid_t h5type);

  template <typename T>
  void appendToDataSet(std::size_t DSindex, T* buf, size_t elements);

  void appendToDataSetRaw(std::size_t DSindex, void* buf, size_t elements,
                          hid_t h5type);

  template <typename T>
  void addRootAttrib(const char* name_arg, const T& val);

  static std::string format_time();

private: // methods
};

#endif // HDF5DATAFILE_H
