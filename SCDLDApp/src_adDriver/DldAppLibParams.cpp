#include "DldAppLibParams.hpp"
#include "dldApp.h"
#include <exception>
#include <string>
#include <memory>

using namespace DldApp;

std::vector<Param> &Lib::params()
{
  return params_;
}

std::unordered_map<std::string, NonDbParam> &Lib::nondb_params()
{
  return nondb_params_;
}

Lib::Lib()
{

  // TODO: create this list from json config string queried from the library
  // -> scdldapp_get_param_config_json();
  // processing JSON is easy using the nlohmann::json library
  // this would guarantee robust mapping to the library parameter indices
  // even if you update the library without recompiling this AD driver.
  params_.emplace_back("DLD_INIT",      asynParamInt32,  DATATYPE_ENUM);
  params_.emplace_back("DLD_CFGFILE",   asynParamOctet,  DATATYPE_STRING);
  nondb_params_.emplace("StatusMessage", NonDbParam(DATATYPE_STRING, 2));
  nondb_params_.emplace("Exposure",  NonDbParam(DATATYPE_FLOAT64, 3));
  nondb_params_.emplace("Acquire",   NonDbParam(DATATYPE_ENUM, 4));
  nondb_params_.emplace("ImageMode", NonDbParam(DATATYPE_ENUM, 5));
  nondb_params_.emplace("NumImages", NonDbParam(DATATYPE_INT32, 6));
  nondb_params_.emplace("DataType",  NonDbParam(DATATYPE_ENUM, 7));
}

int Lib::numberParams() const
{
  return 2; // TODO parse json and count the parameters
}

Lib &Lib::instance()
{
  static Lib the_instance;
  return the_instance;
}

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ------------------------------------------------------------------------ */

LibUser::LibUser()
  : user_id_(-1), first_driver_param_(-1)
{
  int ret = scdldapp_create_user();
  if (ret < 0) {
    throw std::runtime_error("scdldapp_create_user() returned an error");
  }
  user_id_ = ret;
}

LibUser::~LibUser()
{
  scdldapp_delete_user(user_id_);
}

int LibUser::createParams(
    std::function<int (const char *, asynParamType, int *)> f)
{
  auto& dld_params = Lib::instance().params();
  for (std::size_t i = 0; i < dld_params.size(); i++) {
    const Param& p = dld_params[i];
    int asyn_port_param_idx = -1;
    int ret = f(p.ap_name.c_str(), p.asyn_param_type, &asyn_port_param_idx);
    if (ret != 0) {
      return ret;
    }
    if (first_driver_param_ == -1) {
      first_driver_param_ = asyn_port_param_idx;
    }
    param_refs_[asyn_port_param_idx] = i;
    param_back_refs_.push_back(asyn_port_param_idx);
  }
  return 0;
}

int LibUser::writeInt32(int asynport_param_idx, int value)
{
  try {
    auto pidx = ap2lib(asynport_param_idx);
    const Param& p = Lib::instance().params().at(pidx);
    if ( p.lib_param_type == DATATYPE_ENUM) {
      return scdldapp_write_enum(
        user_id_, ap2lib(asynport_param_idx), value);
    }
    else if ( p.lib_param_type == DATATYPE_INT32 ) {
      return scdldapp_write_int32(
        user_id_, ap2lib(asynport_param_idx), value);
    }
    else {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::writeInt32(const std::string &name, int value)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    if (p.datatype == DATATYPE_ENUM) {
      return scdldapp_write_enum(
        user_id_, p.lib_param_idx, value);
    }
    else if (p.datatype == DATATYPE_INT32 ) {
      return scdldapp_write_int32(
        user_id_, p.lib_param_idx, value);
    }
    else {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::writeFloat64(int asynport_param_idx, double value)
{
  try {
    return scdldapp_write_float64(
      user_id_, ap2lib(asynport_param_idx), value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::writeFloat64(const std::string &name, double value)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    if (p.datatype != DATATYPE_FLOAT64) {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    return scdldapp_write_float64(user_id_, p.lib_param_idx, value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::writeString(int asynport_param_idx, const std::string &value)
{
  try {
    return scdldapp_write_string(
      user_id_, ap2lib(asynport_param_idx), value.c_str());
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::writeString(const std::string &name, const std::string &value)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    if (p.datatype != DATATYPE_STRING) {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    return scdldapp_write_string(user_id_, p.lib_param_idx, value.c_str());
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readInt32(int asynport_param_idx, std::function<void (int)> f)
{
  try {
    auto pidx = ap2lib(asynport_param_idx);
    const Param& p = Lib::instance().params().at(pidx);
    int val = 0;
    int ret = 0;
    if ( p.lib_param_type == DATATYPE_ENUM) {
      ret = scdldapp_read_enum(user_id_, pidx, &val);
    }
    else if ( p.lib_param_type == DATATYPE_INT32 ) {
      ret = scdldapp_read_int32(user_id_, pidx, &val);
    }
    else {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    if (ret == 0) { f(val); }
    return ret;
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readInt32(const std::string &name, std::function<void (int)> f)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    int val = 0;
    int ret = 0;
    if (p.datatype == DATATYPE_ENUM) {
      ret = scdldapp_read_enum(user_id_, p.lib_param_idx, &val);
    }
    else if (p.datatype == DATATYPE_INT32) {
      ret = scdldapp_read_int32(user_id_, p.lib_param_idx, &val);
    }
    else {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    if (ret == 0) { f(val); }
    return ret;
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readFloat64(int asynport_param_idx, std::function<void (double)> f)
{
  try {
    double val = 0;
    int ret = scdldapp_read_float64(user_id_, ap2lib(asynport_param_idx), &val);
    if (ret == 0) { f(val); }
    return ret;
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readFloat64(const std::string &name, std::function<void (double)> f)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    if (p.datatype != DATATYPE_FLOAT64) {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    double val = 0;
    int ret = scdldapp_read_float64(user_id_, p.lib_param_idx, &val);
    if (ret == 0) { f(val); }
    return ret;
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readString(
  int asynport_param_idx, std::function<void (const std::string &)> f)
{
  try {
    return readStrImpl(ap2lib(asynport_param_idx), f);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readString(
  const std::string &name, std::function<void (const std::string &)> f)
{
  try {
    NonDbParam& p = Lib::instance().nondb_params().at(name);
    if (p.datatype != DATATYPE_STRING) {
      return DLDAPPLIB_WRONG_DATATYPE;
    }
    return readStrImpl(p.lib_param_idx, f);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int LibUser::readStrImpl(
  int libparidx, std::function<void (const std::string &)> f)
{
  std::size_t len = 0;
  int ret =
    scdldapp_read_string(user_id_, libparidx, &len, nullptr);
  if (ret == 0) {
    std::unique_ptr<char[]> buf(new char[len]);
    scdldapp_read_string(user_id_, libparidx, &len, buf.get());
    std::string val(buf.get());
    f(val);
  }
  return ret;
}

int LibUser::firstDriverParamIdx() const
{
  return first_driver_param_;
}

std::size_t LibUser::ap2lib(int asynport_param_idx) const
{
  return param_refs_.at(asynport_param_idx);
}

std::string LibUser::paramName(int asynport_param_idx) const
{
  try {
    return Lib::instance().params().at(ap2lib(asynport_param_idx)).ap_name;
  } catch (const std::out_of_range&) {
    return "UnknownParameter";
  }
}

const Param &LibUser::libParam(int asynport_param_idx) const
{
  return Lib::instance().params().at(ap2lib(asynport_param_idx));
}
