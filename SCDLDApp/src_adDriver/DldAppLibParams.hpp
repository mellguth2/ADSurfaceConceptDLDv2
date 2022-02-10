#ifndef DLDAPPLIBPARAMS_HPP
#define DLDAPPLIBPARAMS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <asynParamType.h>

#define DLDAPPLIB_NOT_MY_PARAM -820000

struct DldAppParam {
  std::string ap_name;
  asynParamType asyn_param_type;
  DldAppParam(
    const std::string& apname_arg,
    asynParamType asyn_param_type_arg)
    : ap_name(apname_arg),
      asyn_param_type(asyn_param_type_arg)
  { }
};

class DldAppLib // I'm a singleton
{
  friend class DldAppLibUser;
  /* private data */
  std::vector<DldAppParam> dld_params_;
  /* private functions */
  std::vector<DldAppParam>& dld_params();
public:
  DldAppLib();
  int numberParams() const;
  static DldAppLib& instance();
};

/**
 * @brief objects of this class should be made members of the asynPortDriver
 * instance, since they provide the mapping of asynPortDriver parameter IDs to
 * app library parameter IDs and vice versa
 */
class DldAppLibUser
{
  std::unordered_map<int, size_t> dld_param_refs_; // asynPortDrv -> app lib
  std::vector<int> dld_param_back_refs_;           // app lib -> asynPortDrv
  int app_lib_user_id_;
  int first_driver_param_;
public:
  DldAppLibUser();
  ~DldAppLibUser();
  /**
   * @brief createParams create all parameters using provided function f
   * @param f pass in a function that calls ADDriver::createParam and returns 0
   * on success
   * @return the same value that f returned if it was non-zero; zero on success
   */
  int createParams(std::function<int(const char*, asynParamType, int*)> f);
  int writeInt32(int asynport_param_idx, int value);
  int writeFloat64(int asynport_param_idx, double value);
  int firstDriverParamIdx() const;
  std::size_t libidx(int asynport_param_idx) const; // may throw std::out_of_range
  std::string paramName(int asynport_param_idx) const;
};

#endif // DLDAPPLIBPARAMS_HPP
