#pragma once

/* Copyright 2022 Surface Concept GmbH */

#include <cstddef>
#include <asynParamType.h>
#include "DldAppLib.hpp"
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>

/**
 * @brief cache 1D array data received by updates from the libdldApp.so,
 * convert to one of the types supported by asynPortDriver, if necessary.
 * Clients may need to read the array data later than the time of the update,
 * for instance, when they connect at a later point.
 * Throughout the code for libdldDetectorv2.so (the areaDetector driver code),
 * consistently use the following mappings from libdldApp.so array element types
 * (i8,u8,i16,u16,i32,u32,i64,u64,f32,f64) to the asynPortDriver types:
 * i8  -> asynParamInt8Array,
 * u8  -> asynParamInt16Array,     (conversion)
 * i16 -> asynParamInt16Array,
 * u16 -> asynParamInt32Array,     (conversion)
 * i32 -> asynParamInt32Array,
 * u32 -> asynParamFloat64Array,   (conversion)
 * i64 -> asynParamFloat64Array,   (conversion)
 * u64 -> asynParamFloat64Array,   (conversion)
 * f32 -> asynParamFloat32Array,
 * f64 -> asynParamFloat64Array
 * This choice is made to avoid data loss wherever possible (not generally
 * possible for i64 and u64).
 */
class CachedArrays
{
public:
  CachedArrays();
  /**
   * @brief cache data associated to a driver parameter index
   * @param drvpidx
   * @param bytelen
   * @param data
   */
  void updateArray1D(
    std::size_t drvpidx,
    DldApp::ElementDatatypeEnum,
    std::size_t bytelen,
    void* data);

  void updateImage(
    int addr,
    DldApp::ElementDatatypeEnum,
    std::size_t maxlen,
    std::size_t bytelen,
    std::size_t width,
    void* data);

  /**
   * @brief get array data that has been cached by a previous call to
   * updateArray1D()
   * @param drvpidx the parameter index as delivered by asynUser::reason
   * @param arraytype one of the asynParamXYZArray constants
   * @param consumer a function that gets passed the pointer to the data and
   * the size of the data in bytes
   * @return true if array data was available
   */
  bool getArray1D(
    std::size_t drvpidx,
    asynParamType arraytype,
    std::function<void(void*, std::size_t)> consumer);

  // getImage: currently only for Int32 voxels
  // consumer function gets data pointer, width and height
  bool getImage(
    int addr,
    std::function<void(void*, std::size_t, std::size_t)> consumer);

private:
  typedef std::vector<char> i8array;
  typedef std::vector<short> i16array;
  typedef std::vector<int> i32array;
  typedef std::vector<float> f32array;
  typedef std::vector<double> f64array;
  std::unordered_map<std::size_t, i8array> i8arrays;
  std::unordered_map<std::size_t, i16array> i16arrays;
  std::unordered_map<std::size_t, i32array> i32arrays;
  std::unordered_map<std::size_t, f32array> f32arrays;
  std::unordered_map<std::size_t, f64array> f64arrays;
  template <typename T> struct image {
    std::size_t width;
    std::size_t height;
    std::vector<T> data;
  };
  std::unordered_map<int, image<int> > i32images;
  std::mutex mutex_;
  template <DldApp::ElementDatatypeEnum E>
  struct EDTHelper;
  template <DldApp::ElementDatatypeEnum E>
  void updateArray1D_impl(
    std::size_t drvpidx,
    std::size_t bytelen,
    void* data);
};

