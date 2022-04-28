#ifndef DLDAPPLIBPARAMS_HPP
#define DLDAPPLIBPARAMS_HPP

/* Copyright 2022 Surface Concept GmbH */

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <asynParamType.h>
#include "InitLibDataFromJSON.hpp"
#include "UpdateConsumer.hpp"
#include "DldAppLib.hpp"

namespace DldApp
{

/**
 * @brief objects of this class should be made members of the asynPortDriver
 * instance, since they provide the mapping of asynPortDriver parameter IDs to
 * app library parameter IDs and vice versa
 */
class LibUser
{
  std::unordered_map<int, size_t> param_refs_; // asynPortDrv -> app lib
  std::vector<int> param_back_refs_;           // app lib -> asynPortDrv
  int user_id_;
  int first_driver_param_;
  std::unique_ptr<UpdateConsumer> update_consumer_;
public:
  LibUser();
  ~LibUser();
  /**
   * @brief createParams create all parameters using provided function f
   * @param f pass in a function that calls ADDriver::createParam and returns 0
   * on success
   * @return the same value that f returned if it was non-zero; zero on success
   */
  int createParams(std::function<int(const char*, asynParamType, int*)> f);

  /**
   * @brief linkParam
   * @param drvpidx the asynport parameter index
   * @param t the asynport parameter type
   * @param libpname the parameter name of the library to link to
   * @return 0 on success
   */
  int linkParam(int drvpidx, asynParamType t, const std::string& libpname);

  int setUpdateConsumer(std::unique_ptr<UpdateConsumer>&&);
  void resetUpdateConsumer();

  template <typename ValueType>
  int writeAny(int drvpidx, const ValueType& value);

  template <typename ValueType>
  int writeAny(const std::string& name, const ValueType& value);

  template <typename ValueType>
  int readAny(int drvpidx, std::function<void(const ValueType&)>);

  template <typename ValueType>
  int readAny(const std::string& name, std::function<void(const ValueType&)>);

  template <typename ValueType>
  int writeAndReadAny(int drvpidx, const ValueType& value,
                      std::function<void(const ValueType&)>);

  template <typename ValueType>
  int writeAndReadAny(const std::string& name, const ValueType& value,
                      std::function<void(const ValueType&)>);

  int firstDriverParamIdx() const;
  std::size_t ap2lib(int asynport_param_idx) const; // may throw std::out_of_range
  int lib2ap(std::size_t libpidx) const;
  std::string paramName(int asynport_param_idx) const;
  DatatypeEnum libdatatype(int asynport_param_idx) const;
  const Param& libParam(int asynport_param_idx) const;
  int array2d_address(std::size_t libpidx) const;
  ElementDatatypeEnum element_type(std::size_t libpidx) const;
  std::size_t array_maxlength(std::size_t libpidx) const;

private:
  int readStrImpl(int libparidx, std::function<void(const std::string&)>);

  template <typename ValueType>
  int writeAnyImpl(int libpidx, DatatypeEnum, const ValueType&);
  template <typename ValueType>
  int readAnyImpl(int libpidx, DatatypeEnum, std::function<void(const ValueType&)>);

  void cb_int32(size_t, int);
  void cb_float64(size_t, double);
  void cb_string(size_t, const char*);
  void cb_enum(size_t, int);
  void cb_arr1d(size_t, size_t, void*);
  void cb_arr2d(size_t, size_t, size_t, void*);
  static void static_cb_int32(void*, size_t, int);
  static void static_cb_float64(void*, size_t, double);
  static void static_cb_string(void*, size_t, const char*);
  static void static_cb_enum(void*, size_t, int);
  static void static_cb_arr1d(void*, size_t, size_t, void*);
  static void static_cb_arr2d(void*, size_t, size_t, size_t, void*);
};

} // namespace DldApp

#endif // DLDAPPLIBPARAMS_HPP
