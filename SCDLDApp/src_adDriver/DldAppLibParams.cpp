#include "DldAppLibParams.hpp"
#include "dldApp.h"
#include <exception>

std::vector<DldAppParam> &DldAppLib::dld_params()
{
  return dld_params_;
}

DldAppLib::DldAppLib()
{
  // TODO: create this list from json config string queried from the library
  dld_params_.emplace_back("DLD_INIT",      asynParamInt32);
  dld_params_.emplace_back("DLD_CFGFILE",   asynParamOctet);
  dld_params_.emplace_back("DLD_STATUSMSG", asynParamOctet);
  dld_params_.emplace_back("DLD_EXPOSURE",  asynParamFloat64);
  dld_params_.emplace_back("DLD_ACQUIRE",   asynParamInt32);
}

int DldAppLib::numberParams() const
{
  return 5; // TODO parse json and count the parameters
}

DldAppLib &DldAppLib::instance()
{
  static DldAppLib the_instance;
  return the_instance;
}

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ------------------------------------------------------------------------ */

DldAppLibUser::DldAppLibUser()
  : app_lib_user_id_(-1), first_driver_param_(-1)
{
  int ret = scdldapp_create_user();
  if (ret < 0) {
    throw std::runtime_error("scdldapp_create_user() returned an error");
  }
  app_lib_user_id_ = ret;
}

DldAppLibUser::~DldAppLibUser()
{
  scdldapp_delete_user(app_lib_user_id_);
}

int DldAppLibUser::createParams(
    std::function<int (const char *, asynParamType, int *)> f)
{
  auto& dld_params = DldAppLib::instance().dld_params();
  for (std::size_t i = 0; i < dld_params.size(); i++) {
    const DldAppParam& p = dld_params[i];
    int asyn_port_param_idx = -1;
    int ret = f(p.ap_name.c_str(), p.asyn_param_type, &asyn_port_param_idx);
    if (ret != 0) {
      return ret;
    }
    if (first_driver_param_ == -1) {
      first_driver_param_ = asyn_port_param_idx;
    }
    dld_param_refs_[asyn_port_param_idx] = i;
    dld_param_back_refs_.push_back(asyn_port_param_idx);
  }
  return 0;
}

int DldAppLibUser::writeInt32(int asynport_param_idx, int value)
{
  try {
    return scdldapp_write_int32(
      app_lib_user_id_, libidx(asynport_param_idx), value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int DldAppLibUser::writeFloat64(int asynport_param_idx, double value)
{
  try {
    return scdldapp_write_float64(
      app_lib_user_id_, libidx(asynport_param_idx), value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

int DldAppLibUser::firstDriverParamIdx() const
{
  return first_driver_param_;
}

std::size_t DldAppLibUser::libidx(int asynport_param_idx) const
{
  return dld_param_refs_.at(asynport_param_idx);
}

std::string DldAppLibUser::paramName(int asynport_param_idx) const
{
  return
    DldAppLib::instance().dld_params().at(libidx(asynport_param_idx)).ap_name;
}

