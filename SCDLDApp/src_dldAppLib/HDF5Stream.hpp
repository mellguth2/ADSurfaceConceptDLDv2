#pragma once

#include <string>
#include "iCreatedAtInit.hpp"
#include "iDisconnectListener.hpp"

class HDF5Stream : public iCreatedAtInit, public iDisconnectListener
{
public:
  HDF5Stream();
  virtual ~HDF5Stream();
  int create(int dev_desc) override;
  void disconnect() override;
  void setFilePath(const std::string&);
  std::string filePath() const;
  void setUserComment(const std::string&);
  std::string userComment() const;
  int setActive(int);
  int isActive() const;
  int fileError() const;

private:
  int hdf5obj_ = -1;
  int active_ = 0;
  int file_error_ = 0;
  std::string filepath_;
  std::string comment_;
};
