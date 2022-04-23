#pragma once

#include <cstddef>
#include <string>

namespace DldApp
{

class UpdateConsumer
{
public:
  virtual ~UpdateConsumer() {}
  virtual void UpdateInt32(std::size_t libpidx, int) = 0;
  virtual void UpdateFloat64(std::size_t libpidx, double) = 0;
  virtual void UpdateString(std::size_t libpidx, const std::string&) = 0;
  virtual void UpdateArray1D(std::size_t libpidx, std::size_t bytelen, void* data) = 0;
  virtual void UpdateArray2D(std::size_t libpidx, std::size_t bytelen, std::size_t width, void* data) = 0;
};

} // namespace DldApp
