#pragma once

#include "UpdateConsumer.hpp"
#include "CachedArrays.hpp"

class dldDetectorv2;

class ADUpdateConsumer : public DldApp::UpdateConsumer {
  dldDetectorv2* parent_;
  std::unique_ptr<CachedArrays> arrays_;
  /*
  std::unordered_map<size_t, int> to_ndarray; // maps indices from lib parameter to NDArray
  */
public:
  ADUpdateConsumer(dldDetectorv2* parent);
  ~ADUpdateConsumer() {}
  virtual void UpdateInt32(std::size_t libpidx, int val) override;
  virtual void UpdateFloat64(std::size_t libpidx, double val) override;
  virtual void UpdateString(std::size_t libpidx, const std::string& val) override;
  virtual void UpdateArray1D(
    std::size_t libpidx, std::size_t bytelen, void* data) override;
  virtual void UpdateArray2D(
    std::size_t libpidx, std::size_t bytelen, std::size_t width,
    void* data) override; // TODO support for images

  CachedArrays& arrays() { return *arrays_; }
};
