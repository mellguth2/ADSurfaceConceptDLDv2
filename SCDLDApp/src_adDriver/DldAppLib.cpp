#include "DldAppLib.hpp"

using namespace DldApp;

Lib::Lib()
  : nr_driver_params_(0), nr_array2d_params_(0)
{
  // create the in-memory parameter configuration from a json config string
  // that is queried from the library (scdldapp_get_param_config_json())
  // this guarantees robust mapping to the library parameter indices
  // even if you update the library without recompiling this AD driver.
  init_libdata_from_json(*this);
  // this has a similar effect as the commented-out code below, except the
  // name-to-index mapping is less prone to being faulty
  /*
  params_.emplace_back(DATATYPE_ENUM, "DLD_INIT", asynParamInt32);
  name2libidx_.insert("Initialize", 0);

  params_.emplace_back(DATATYPE_STRING, "DLD_CFGFILE", asynParamOctet);
  name2libidx_.insert("ConfigFile", 1);

  params_.emplace_back(DATATYPE_STRING);
  name2libidx_.insert("StatusMessage", 2);

  params_.emplace_back(DATATYPE_FLOAT64);
  name2libidx_.insert("Exposure", 3);

  params_.emplace_back(DATATYPE_ENUM);
  name2libidx_.insert("Acquire", 4);

  params_.emplace_back(DATATYPE_ENUM);
  name2libidx_.insert("ImageMode", 5);

  params_.emplace_back(DATATYPE_INT32);
  name2libidx_.insert("NumImages", 6);

  params_.emplace_back(DATATYPE_ENUM);
  name2libidx_.insert("DataType", 7);
  */
}

int Lib::numberDrvParams() const
{
  return nr_driver_params_;
}

int Lib::numberArray2dParams() const
{
  return nr_array2d_params_;
}

bool Lib::hasParamName(const std::string& name) const
{
  return name2libidx_.find(name) != name2libidx_.end();
}

std::size_t Lib::idxFromParamName(const std::string& name) const
{
  try {
    return name2libidx_.at(name);
  } catch (const std::out_of_range&) {
    return static_cast<std::size_t>(-1); // evil, std::optional would be good,
    // however, i'm avoiding c++14 or higher for AD driver code
  }
}

asynParamType Lib::drvType(const Param& p)
{
  asynParamType drvtype = asynParamNotDefined;
  if (p.lib_type == DATATYPE_ARRAY1D) {
    static std::unordered_map<ElementDatatypeEnum, asynParamType> m2;
    if (m2.empty()) {
      m2[ELEMTYPE_I8] = asynParamInt8Array;
      m2[ELEMTYPE_U8] = asynParamInt16Array;
      m2[ELEMTYPE_I16] = asynParamInt16Array;
      m2[ELEMTYPE_U16] = asynParamInt32Array;
      m2[ELEMTYPE_I32] = asynParamInt32Array;
      m2[ELEMTYPE_U32] = asynParamFloat64Array;
      m2[ELEMTYPE_I64] = asynParamFloat64Array;
      m2[ELEMTYPE_U64] = asynParamFloat64Array;
      m2[ELEMTYPE_F32] = asynParamFloat32Array;
      m2[ELEMTYPE_F64] = asynParamFloat64Array;
    }
    try {
      drvtype = m2.at(p.arr_cfg->elemtype);
    } catch (const std::out_of_range&) { }
  }
  else {
    static std::unordered_map<DatatypeEnum, asynParamType> m;
    if (m.empty()) {
      m[DATATYPE_ENUM] = asynParamInt32;
      m[DATATYPE_INT32] = asynParamInt32;
      m[DATATYPE_FLOAT64] = asynParamFloat64;
      m[DATATYPE_STRING] = asynParamOctet;
      m[DATATYPE_ARRAY2D] = asynParamGenericPointer;
    }
    try {
      drvtype = m.at(p.lib_type);
    } catch (const std::out_of_range&) { }
  }
  return drvtype;
}

std::vector<Param> &Lib::params()
{
  return params_;
}

const std::vector<Param>& Lib::getParams()
{
  return params_;
}

int Lib::configureParam(
  const std::string& libpname, asynParamType t, std::function<void (size_t)> f)
{
  try {
    auto& libidx = name2libidx_.at(libpname);
    params_.at(libidx).drv_type = t;
    f(libidx);
    return 0;
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

Lib &Lib::instance()
{
  static Lib the_instance;
  return the_instance;
}
