/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#include "HDF5DataFile.hpp"

#include <time.h>
#include <H5Cpp.h>
#include <H5DOpublic.h>
#include <H5Exception.h>
#include "HDF5Utilities.h"
#include "HDF5Config.hpp"

class H5P_Owner { // raii wrapper for HDF5 property list resource
  hid_t p_id_;
public:
  H5P_Owner(hid_t arg) : p_id_(arg) { if (arg < 0) throw arg; }
  H5P_Owner(const H5P_Owner&) = delete; // no copy constructor allowed
  H5P_Owner& operator=(const H5P_Owner&) = delete; // no assignment allowed
  void close() { if (p_id_ < 0) return; H5Pclose(p_id_); p_id_ = -1; }
  ~H5P_Owner() { close(); }
  hid_t id() { if (p_id_ < 0) throw p_id_; return p_id_; }
};

HDF5DataFile::HDF5DataFile()
{

}

void HDF5DataFile::open(const std::string& fpath)
{
  close(); // close if we still have an open file
  // ----------------------------------------------------------------------
  {
    H5P_Owner fapl(H5Pcreate(H5P_FILE_ACCESS)); // create property list
    //H5Pset_libver_bounds(fapl.id(), H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    H5Pset_cache(fapl.id(), 0, 0, 0, 0);
    try {
      f_.reset(
        new H5::H5File(fpath.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl.id()));
    }
    catch (const H5::FileIException&) {
      return;
    }
  }
  // ----------------------------------------------------------------------
  // descriptive attributes
  H5::Group g_root = f_->openGroup("/");
  WriteHDF5Attribute(g_root, "Date", format_time());
  // ----------------------------------------------------------------------

}

void HDF5DataFile::close()
{
  if (f_) {
    f_->close();
    f_.reset();
  }
}

bool HDF5DataFile::isOpen() const
{
  return static_cast<bool>(f_);
}

HDF5DataFile::~HDF5DataFile()
{
  close();
}

void HDF5DataFile::closeDataSets()
{
  for (std::size_t i = 0; i < datasets_.size(); i++) {
    if (datasets_[i])
      datasets_[i]->close();
  }
  datasets_.clear();
}

std::size_t HDF5DataFile::addDataSetRaw(const char *name_arg, hid_t h5type)
{
  hsize_t dims[RANK];
  hsize_t maxdims[RANK];

  dims[0] = 0;
  maxdims[0] = H5S_UNLIMITED;
  H5::DataSpace dataSpace = H5::DataSpace(RANK, dims, maxdims);

  H5P_Owner dapl(H5Pcreate(H5P_DATASET_ACCESS)); // property list
  H5Pset_chunk_cache(dapl.id(), 0, 0, H5D_CHUNK_CACHE_W0_DEFAULT);
  H5P_Owner lcpl(H5Pcreate(H5P_LINK_CREATE)); // property list
  // we don't set any properties in lcpl, but it is required by H5Dcreate
  // ------------------------
  H5::DSetCreatPropList prop;
  hsize_t chunkdims[RANK];
  chunkdims[0] = EVENTS_PER_CHUNK;
  prop.setChunk(RANK, chunkdims);
  if (PACKLEVEL>0)
    prop.setDeflate(PACKLEVEL);
  // ------------------------

  hid_t datasetc = H5Dcreate(
    f_->getId(), name_arg, h5type, dataSpace.getId(),
    lcpl.id(), prop.getId(), dapl.id());
  // copy and convert to C++
  datasets_.push_back(std::make_shared<H5::DataSet>(datasetc));
  H5Dclose(datasetc);
  return datasets_.size()-1;
}

template <typename T>
std::size_t HDF5DataFile::addDataSet(const char* name_arg)
{
  return addDataSetRaw(name_arg, GetH5DataType(T()).getId());
}

// explicit template instantiations
template
std::size_t HDF5DataFile::addDataSet<unsigned short>(const char* name_arg);

template
std::size_t HDF5DataFile::addDataSet<unsigned long long>(const char* name_arg);


// -----------------------------------------------------------------------------
template <typename T>
void HDF5DataFile::appendToDataSet(std::size_t DSindex, T* buf, size_t elements)
{
  H5DOappend(datasets_[DSindex]->getId(), H5P_DEFAULT, 0, elements,
             GetH5DataType(T()).getId(), buf);
}
// explicit template instantiations
template
void HDF5DataFile::appendToDataSet<unsigned short>(
  std::size_t, unsigned short*, size_t);
template
void HDF5DataFile::appendToDataSet<unsigned long long>(
  std::size_t, unsigned long long*, size_t);

// -----------------------------------------------------------------------------

void HDF5DataFile::appendToDataSetRaw(std::size_t DSindex, void* buf,
                                      size_t elements, hid_t h5type)
{
  if (h5type < 0) return;
  H5DOappend(
    datasets_[DSindex]->getId(), H5P_DEFAULT, 0, elements, h5type, buf);
}

// -----------------------------------------------------------------------------

std::string HDF5DataFile::format_time()
{
  char date[32];
  time_t t = time(0);
  struct tm *tm;
  tm = gmtime(&t);
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);
  return std::string(date);
}

// -----------------------------------------------------------------------------

template <typename T>
void HDF5DataFile::addRootAttrib(const char *name_arg, const T& val)
{
  H5::Group g_root = f_->openGroup("/");
  WriteHDF5Attribute(g_root, name_arg, val);
}

// explicit template instantiations
template
void HDF5DataFile::addRootAttrib<std::string>(const char*, const std::string&);
