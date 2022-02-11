#ifndef DLDAPPLIBPARAMS_HPP
#define DLDAPPLIBPARAMS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <asynParamType.h>

#define DLDAPPLIB_NOT_MY_PARAM   -820000
#define DLDAPPLIB_WRONG_DATATYPE -820001

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
  std::string ap_name;
  asynParamType asyn_param_type;
  DatatypeEnum lib_param_type;
  Param(
    const std::string& apname_arg,
    asynParamType asyn_param_type_arg,
    DatatypeEnum lib_param_type_arg)
    : ap_name(apname_arg),
      asyn_param_type(asyn_param_type_arg),
      lib_param_type(lib_param_type_arg)
  { }
};

struct NonDbParam {
  DatatypeEnum datatype;
  int lib_param_idx;
  NonDbParam(
    DatatypeEnum datatype_arg,
    int lib_param_idx_arg)
    : datatype(datatype_arg),
      lib_param_idx(lib_param_idx_arg)
  { }
};

class Lib // I'm a singleton
{
  friend class LibUser;
  /* private data */
  std::vector<Param> params_;
  std::unordered_map<std::string, NonDbParam> nondb_params_;
  /* private functions */
  std::vector<Param>& params();
  std::unordered_map<std::string, NonDbParam>& nondb_params();

public:
  Lib();
  int numberParams() const;
  static Lib& instance();
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
  int writeInt32(int asynport_param_idx, int value); // for int32 and enums
  int writeInt32(const std::string& name, int value); // for int32 and enums
  int writeFloat64(int asynport_param_idx, double value);
  int writeFloat64(const std::string& name, double value);
  int writeString(int asynport_param_idx, const std::string& value);
  int writeString(const std::string& name, const std::string& value);
  int readInt32(int asynport_param_idx, std::function<void(int)>); // for int32 and enums
  int readInt32(const std::string& name, std::function<void(int)>); // for int32 and enums
  int readFloat64(int asynport_param_idx, std::function<void(double)>);
  int readFloat64(const std::string& name, std::function<void(double)>);
  int readString(int asynport_param_idx, std::function<void(const std::string&)>);
  int readString(const std::string& name, std::function<void(const std::string&)>);
  int firstDriverParamIdx() const;
  std::size_t ap2lib(int asynport_param_idx) const; // may throw std::out_of_range
  std::string paramName(int asynport_param_idx) const;
  DatatypeEnum libdatatype(int asynport_param_idx) const;
  const Param& libParam(int asynport_param_idx) const;

private:
  int readStrImpl(int libparidx, std::function<void(const std::string&)>);
};

} // namespace DldApp

#endif // DLDAPPLIBPARAMS_HPP
