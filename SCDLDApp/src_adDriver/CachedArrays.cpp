#include "CachedArrays.hpp"
#include <cstring>

// TODO: handle all element data types, some of which need conversion to one
// of the supported asynPortDriver array types

CachedArrays::CachedArrays()
{

}

namespace {
  template <DldApp::ElementDatatypeEnum E>
  struct toType { };
  template <> struct toType<DldApp::ELEMTYPE_I8> { typedef char t; };
  template <> struct toType<DldApp::ELEMTYPE_I16> { typedef short t; };
  template <> struct toType<DldApp::ELEMTYPE_I32> { typedef int t; };
  template <> struct toType<DldApp::ELEMTYPE_F32> { typedef float t; };
  template <> struct toType<DldApp::ELEMTYPE_F64> { typedef double t; };
}

template <DldApp::ElementDatatypeEnum E>
struct CachedArrays::EDTHelper
{
};
template <>
struct CachedArrays::EDTHelper<DldApp::ELEMTYPE_I8>
{
  CachedArrays& i;
  EDTHelper(CachedArrays& i) : i(i) {}
  std::unordered_map<std::size_t, i8array>& arrays() { return i.i8arrays; }
};
template <>
struct CachedArrays::EDTHelper<DldApp::ELEMTYPE_I16>
{
  CachedArrays& i;
  EDTHelper(CachedArrays& i) : i(i) {}
  std::unordered_map<std::size_t, i16array>& arrays() { return i.i16arrays; }
};
template <>
struct CachedArrays::EDTHelper<DldApp::ELEMTYPE_I32>
{
  CachedArrays& i;
  EDTHelper(CachedArrays& i) : i(i) {}
  std::unordered_map<std::size_t, i32array>& arrays() { return i.i32arrays; }
};
template <>
struct CachedArrays::EDTHelper<DldApp::ELEMTYPE_F32>
{
  CachedArrays& i;
  EDTHelper(CachedArrays& i) : i(i) {}
  std::unordered_map<std::size_t, f32array>& arrays() { return i.f32arrays; }
};
template <>
struct CachedArrays::EDTHelper<DldApp::ELEMTYPE_F64>
{
  CachedArrays& i;
  EDTHelper(CachedArrays& i) : i(i) {}
  std::unordered_map<std::size_t, f64array>& arrays() { return i.f64arrays; }
};



template<DldApp::ElementDatatypeEnum E>
void CachedArrays::updateArray1D_impl(
  std::size_t drvpidx, std::size_t bytelen, void* data)
{
  auto nr_elements = bytelen / sizeof(typename toType<E>::t);
  EDTHelper<E> h(*this);
  try {
    auto& v = h.arrays().at(drvpidx);
    v.resize(nr_elements);
    memcpy(v.data(), data, bytelen);
  }
  catch (const std::out_of_range&) {
    h.arrays()[drvpidx] = {};
    auto& v = h.arrays().at(drvpidx);
    v.resize(nr_elements);
    memcpy(v.data(), data, bytelen);
  }
}

void CachedArrays::updateArray1D(
  std::size_t drvpidx,
  DldApp::ElementDatatypeEnum elementtype,
  std::size_t bytelen,
  void* data)
{
  std::lock_guard<std::mutex> l(mutex_);
  switch (elementtype) {
  case DldApp::ELEMTYPE_I8:
    updateArray1D_impl<DldApp::ELEMTYPE_I8>(drvpidx, bytelen, data);
    break;
  case DldApp::ELEMTYPE_I16:
    updateArray1D_impl<DldApp::ELEMTYPE_I16>(drvpidx, bytelen, data);
    break;
  case DldApp::ELEMTYPE_I32:
    updateArray1D_impl<DldApp::ELEMTYPE_I32>(drvpidx, bytelen, data);
    break;
  case DldApp::ELEMTYPE_F32:
    updateArray1D_impl<DldApp::ELEMTYPE_F32>(drvpidx, bytelen, data);
    break;
  case DldApp::ELEMTYPE_F64:
    updateArray1D_impl<DldApp::ELEMTYPE_F64>(drvpidx, bytelen, data);
    break;
  default:
    break;
  }
}

void CachedArrays::updateImage(
  int addr, DldApp::ElementDatatypeEnum elementtype,
  std::size_t maxlength, std::size_t bytelen, std::size_t width, void* data)
{
  std::lock_guard<std::mutex> l(mutex_);
  switch (elementtype) {
  case DldApp::ELEMTYPE_I32:
    {
      auto length = bytelen / sizeof(int);
      if (i32images.find(addr)==i32images.end()) {
        i32images[addr] = {};
        i32images[addr].data.reserve(maxlength);
        // we must prevent that the memory buffer of the vector is ever
        // relocated, once we passed the data pointer to an NDArray
        // this would happen, if the resize
      }
      if (length > maxlength) {
        length = maxlength;
      }
      image<int>& img = i32images.at(addr);
      img.data.resize(length);
      img.width = width;
      img.height = img.data.size() / std::max(width, std::size_t{1u});
      memcpy(img.data.data(), data, bytelen);
    }
    break;
  default:
    break;
  }

}

bool CachedArrays::getArray1D(
  std::size_t drvpidx,
  asynParamType arraytype,
  std::function<void (void*, std::size_t)> f)
{
  std::lock_guard<std::mutex> l(mutex_);
  try {
    if (arraytype == asynParamInt8Array) {
      auto& v = i8arrays.at(drvpidx);
      f(v.data(), v.size() * sizeof(char));
      return true;
    }
    if (arraytype == asynParamInt16Array) {
      auto& v = i16arrays.at(drvpidx);
      f(v.data(), v.size() * sizeof(short));
      return true;
    }
    if (arraytype == asynParamInt32Array) {
      auto& v = i32arrays.at(drvpidx);
      f(v.data(), v.size() * sizeof(int));
      return true;
    }
    else if (arraytype == asynParamFloat32Array) {
      auto& v = f32arrays.at(drvpidx);
      f(v.data(), v.size() * sizeof(float));
      return true;
    }
    else if (arraytype == asynParamFloat64Array) {
      auto& v = f64arrays.at(drvpidx);
      f(v.data(), v.size() * sizeof(double));
      return true;
    }
  }
  catch (const std::out_of_range&) { }
  return false;
}

bool CachedArrays::getImage(
  int addr, std::function<void (void*, std::size_t, std::size_t)> f)
{
  std::lock_guard<std::mutex> l(mutex_);
  try {
    auto& img = i32images.at(addr);
    f(img.data.data(), img.width, img.height);
    return true;
  }
  catch (const std::out_of_range&) { }
  return false;
}
