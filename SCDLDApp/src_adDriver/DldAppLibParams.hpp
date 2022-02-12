#ifndef DLDAPPLIBPARAMS_HPP
#define DLDAPPLIBPARAMS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <asynParamType.h>
#include "InitLibDataFromJSON.hpp"

#define DLDAPPLIB_NOT_MY_PARAM    -820000
#define DLDAPPLIB_WRONG_DATATYPE  -820001
#define DLDAPPLIB_NOT_IMPLEMENTED -820002

namespace DldApp
{
  enum DatatypeEnum {
    DATATYPE_INVALID = 0,
    DATATYPE_ENUM = 1,
    DATATYPE_INT32 = 2,
    DATATYPE_FLOAT64 = 3,
    DATATYPE_STRING = 4
  };

struct Param {
  DatatypeEnum lib_type;  // data type in application library
  std::string drv_name;   // parameter name in the INP/OUT fields of
                          // dldDetectorv2.template, empty string if not ours
  asynParamType drv_type = asynParamNotDefined; // for manual linking of parameters, only
  Param(
    DatatypeEnum lib_type_arg,
    const std::string& drv_name_arg = "")
    : lib_type(lib_type_arg),
      drv_name(drv_name_arg)
  { }
};

class Lib // I'm a singleton
{
  friend class LibUser;
  friend int init_libdata_from_json(Lib&);
  /* private data */
  std::vector<Param> params_;
  std::unordered_map<std::string, std::size_t> name2libidx_;
  int nr_driver_params_;
  /* private functions */
  std::vector<Param>& params();

public:

  int numberDrvParams() const;
  bool hasParamName(const std::string&) const;
  std::size_t idxFromParamName(const std::string&) const;

  /**
   * @brief (1) set the asynParamType for a library parameter and (2) call
   * a function with the library parameter index, (1) and (2) only happen,
   * if the parameter exists.
   * @return 0 on success, else DLDAPPLIB_NOT_MY_PARAM
   */
  int configureParam(const std::string&, asynParamType, std::function<void(size_t)>);
  static Lib& instance();
private:
  Lib();
  Lib(Lib const&) = delete;
  void operator=(Lib const&) = delete;
};

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
  std::string paramName(int asynport_param_idx) const;
  DatatypeEnum libdatatype(int asynport_param_idx) const;
  const Param& libParam(int asynport_param_idx) const;


private:
  int readStrImpl(int libparidx, std::function<void(const std::string&)>);

  template <typename ValueType>
  int writeAnyImpl(int libpidx, DatatypeEnum, const ValueType&);
  template <typename ValueType>
  int readAnyImpl(int libpidx, DatatypeEnum, std::function<void(const ValueType&)>);
};

} // namespace DldApp

#endif // DLDAPPLIBPARAMS_HPP
