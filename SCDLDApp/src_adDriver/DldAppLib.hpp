#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include <asynParamType.h>
#include "DldAppCommon.hpp"
#include "InitLibDataFromJSON.hpp"

#define DLDAPPLIB_NOT_MY_PARAM    -820000
#define DLDAPPLIB_WRONG_DATATYPE  -820001
#define DLDAPPLIB_NOT_IMPLEMENTED -820002

namespace DldApp
{

class Lib // I'm a singleton
{
  friend class LibUser;
  friend int init_libdata_from_json(Lib&);
  /* private data */
  std::vector<Param> params_; // index of vector = library parameter index
  std::unordered_map<std::string, std::size_t> name2libidx_;
  int nr_driver_params_;
  /* private functions */
  std::vector<Param>& params();

public:
  int numberDrvParams() const;
  bool hasParamName(const std::string&) const;
  std::size_t idxFromParamName(const std::string&) const;
  static asynParamType drvType(const Param&);
  const std::vector<Param>& getParams();

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

} // namespace DldApp

