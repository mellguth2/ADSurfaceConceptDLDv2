#ifndef DLDAPPLIBPARAMS_HPP
#define DLDAPPLIBPARAMS_HPP

#include <string>
#include <vector>
#include <memory>
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
  DATATYPE_STRING = 4,
  DATATYPE_ARRAY1D = 5,
  DATATYPE_ARRAY2D = 6
};
enum ElementDatatypeEnum {
  ELEMTYPE_INVALID = 0,
  ELEMTYPE_U8 = 0x01,   // unsigned integer
  ELEMTYPE_I8 = 0x11,   // signed integer
  ELEMTYPE_U16 = 0x02,
  ELEMTYPE_I16 = 0x12,
  ELEMTYPE_U32 = 0x03,
  ELEMTYPE_I32 = 0x13,
  ELEMTYPE_U64 = 0x04,
  ELEMTYPE_I64 = 0x14,
  ELEMTYPE_F32 = 0x23,  // C float
  ELEMTYPE_F64 = 0x24   // C double
};
inline size_t elementSize(ElementDatatypeEnum e) {
  if (e == ELEMTYPE_INVALID)
    return 0;
  else
    return (1u << ((static_cast<unsigned>(e) & 0x7) - 1));
}

struct ArrayParam {
  ArrayParam(ElementDatatypeEnum e, std::size_t l)
    : elemtype(e), maxlength(l) {}
  ElementDatatypeEnum elemtype;  // C type for the elements of the array
  std::size_t maxlength;
};

struct Param {
  DatatypeEnum lib_type;  // data type in application library
  std::string drv_name;   // parameter name in the INP/OUT fields of
                          // dldDetectorv2.template, empty string if not ours
  asynParamType drv_type = asynParamNotDefined; // for manual linking of parameters, only
  std::unique_ptr<ArrayParam> arr_cfg;
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
  std::vector<Param> params_; // index of vector = library parameter index
  std::vector<ArrayParam> array_params_;
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
