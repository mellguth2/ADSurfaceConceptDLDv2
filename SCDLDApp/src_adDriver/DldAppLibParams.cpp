#include "DldAppLibParams.hpp"
#include "dldApp.h"
#include <exception>
#include <string>
#include <memory>
#include <unordered_map>

using namespace DldApp;

std::vector<Param> &Lib::params()
{
  return params_;
}
/*
std::unordered_map<std::string, NonDbParam> &Lib::nondb_params()
{
  return nondb_params_;
}
*/
Lib::Lib()
  : nr_driver_params_(0)
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
  scdldapp_set_callback_enum(user_id_, this, static_cb_enum);
  scdldapp_set_callback_int32(user_id_, this, static_cb_int32);
  scdldapp_set_callback_float64(user_id_, this, static_cb_float64);
  scdldapp_set_callback_string(user_id_, this, static_cb_string);
}

LibUser::~LibUser()
{
  scdldapp_delete_user(user_id_);
}

int LibUser::createParams(
    std::function<int (const char *, asynParamType, int *)> f)
{
  static std::unordered_map<DatatypeEnum, asynParamType> m;
  if (m.empty()) {
    m[DATATYPE_ENUM] = asynParamInt32;
    m[DATATYPE_INT32] = asynParamInt32;
    m[DATATYPE_FLOAT64] = asynParamFloat64;
    m[DATATYPE_STRING] = asynParamOctet;
  }
  auto& dld_params = Lib::instance().params();
  for (std::size_t i = 0; i < dld_params.size(); i++) {
    const Param& p = dld_params[i];
    int asyn_port_param_idx = -1;
    asynParamType drvtype = asynParamNotDefined;
    try {
      drvtype = m.at(p.lib_type);
    } catch (const std::out_of_range&) {
      throw std::runtime_error("LibUser::createParams(...) unsupported type");
    }
    if (p.drv_name.empty()) {
      // we don't have our own driver parameter for this library parameter.
      // The index into "param_back_refs_" vector is the library param index,
      // so we have to push a placeholder (-1) into the vector.
      // Afterwards, the user can "linkParam()" pre-defined ADDriver parameters,
      // to replace the -1 with an actual asynport parameter index
      param_back_refs_.push_back(-1);
      continue;
    }
    int ret = f(p.drv_name.c_str(), drvtype, &asyn_port_param_idx);
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

int LibUser::linkParam(int drvpidx, asynParamType t, const std::string& libpname)
{
  return Lib::instance().configureParam(
    libpname, t, [&](std::size_t libpidx) {
      param_refs_[drvpidx] = libpidx;
      param_back_refs_.at(libpidx) = drvpidx;
  });
}

int LibUser::setUpdateConsumer(std::unique_ptr<UpdateConsumer>&& c)
{
  update_consumer_ = std::move(c);
  return 0;
}

void LibUser::resetUpdateConsumer()
{
  update_consumer_.reset();
}

template <typename ValueType>
int LibUser::writeAnyImpl(int, DatatypeEnum, const ValueType&)
{
  return DLDAPPLIB_NOT_IMPLEMENTED;
}
template <>
int LibUser::writeAnyImpl(int pidx, DatatypeEnum dt, const int& value)
{
  if (dt == DATATYPE_ENUM) {
    return scdldapp_write_enum(user_id_, pidx, value);
  }
  else if (dt == DATATYPE_INT32) {
    return scdldapp_write_int32(user_id_, pidx, value);
  }
  else {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
template <>
int LibUser::writeAnyImpl(int pidx, DatatypeEnum dt, const double& value)
{
  if (dt == DATATYPE_FLOAT64) {
    return scdldapp_write_float64(user_id_, pidx, value);
  }
  else {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
template <>
int LibUser::writeAnyImpl(int pidx, DatatypeEnum dt, const std::string& value)
{
  if (dt == DATATYPE_STRING) {
    return scdldapp_write_string(user_id_, pidx, value.c_str());
  }
  else {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}

template<typename ValueType>
int LibUser::writeAny(int drvpidx, const ValueType& value)
{
  try {
    auto pidx = ap2lib(drvpidx);
    const Param& p = Lib::instance().params().at(pidx);
    return writeAnyImpl(pidx, p.lib_type, value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
// explicit instantiations
template int LibUser::writeAny(int drvpidx, const int& value);
template int LibUser::writeAny(int drvpidx, const double& value);
template int LibUser::writeAny(int drvpidx, const std::string& value);

template<typename ValueType>
int LibUser::writeAny(const std::string& name, const ValueType& value)
{
  try {
    auto pidx = Lib::instance().name2libidx_[name];
    const Param& p = Lib::instance().params().at(pidx);
    return writeAnyImpl(pidx, p.lib_type, value);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
// explicit instantiations
template int LibUser::writeAny(const std::string& name, const int& value);
template int LibUser::writeAny(const std::string& name, const double& value);
template int LibUser::writeAny(const std::string& name, const std::string& value);

template <typename ValueType>
int LibUser::readAnyImpl(int, DatatypeEnum, std::function<void(const ValueType&)>)
{
  return DLDAPPLIB_NOT_IMPLEMENTED;
}
template <>
int LibUser::readAnyImpl(int pidx, DatatypeEnum dt, std::function<void(const int&)> f)
{
  int val = 0;
  int ret = 0;
  if ( dt == DATATYPE_ENUM) {
    ret = scdldapp_read_enum(user_id_, pidx, &val);
  }
  else if ( dt == DATATYPE_INT32 ) {
    ret = scdldapp_read_int32(user_id_, pidx, &val);
  }
  else {
    return DLDAPPLIB_WRONG_DATATYPE;
  }
  if (ret == 0) { f(val); }
  return ret;
}
template <>
int LibUser::readAnyImpl(int pidx, DatatypeEnum dt, std::function<void(const double&)> f)
{
  if (dt != DATATYPE_FLOAT64) {
    return DLDAPPLIB_WRONG_DATATYPE;
  }
  double val = 0;
  int ret = scdldapp_read_float64(user_id_, pidx, &val);
  if (ret == 0) { f(val); }
  return ret;
}
template <>
int LibUser::readAnyImpl(int pidx, DatatypeEnum dt, std::function<void(const std::string&)> f)
{
  if (dt != DATATYPE_STRING) {
    return DLDAPPLIB_WRONG_DATATYPE;
  }
  return readStrImpl(pidx, f);
}

template<typename ValueType>
int LibUser::readAny(int drvpidx, std::function<void(const ValueType&)> f)
{
  try {
    auto pidx = ap2lib(drvpidx);
    const Param& p = Lib::instance().params().at(pidx);
    return readAnyImpl(pidx, p.lib_type, f);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
// explicit instantiations
template int LibUser::readAny(int drvpidx, std::function<void(const int&)> f);
template int LibUser::readAny(int drvpidx, std::function<void(const double&)> f);
template int LibUser::readAny(int drvpidx, std::function<void(const std::string&)> f);

template<typename ValueType>
int LibUser::readAny(const std::string& name, std::function<void(const ValueType&)> f)
{
  try {
    auto pidx = Lib::instance().name2libidx_[name];
    const Param& p = Lib::instance().params().at(pidx);
    return readAnyImpl(pidx, p.lib_type, f);
  } catch (const std::out_of_range&) {
    return DLDAPPLIB_NOT_MY_PARAM;
  }
}
// explicit instantiations
template int LibUser::readAny(const std::string& name, std::function<void(const int&)> f);
template int LibUser::readAny(const std::string& name, std::function<void(const double&)> f);
template int LibUser::readAny(const std::string& name, std::function<void(const std::string&)> f);

template <typename ValueType>
int LibUser::writeAndReadAny(int drvpidx, const ValueType& value,
                    std::function<void(const ValueType&)> f)
{
  int retw = writeAny(drvpidx, value);
  readAny(drvpidx, f);
  return retw;
}

template <typename ValueType>
int LibUser::writeAndReadAny(const std::string& name, const ValueType& value,
                    std::function<void(const ValueType&)> f)
{
  int retw = writeAny(name, value);
  readAny(name, f);
  return retw;
}
/* explicit instantiations: pulls in all the dependent writeAny / readAny */
template
int LibUser::writeAndReadAny(
  int drvpidx, const int& value, std::function<void(const int&)> f);
template
int LibUser::writeAndReadAny(
  int drvpidx, const double& value, std::function<void(const double&)> f);
template
int LibUser::writeAndReadAny(
  int drvpidx, const std::string& value, std::function<void(const std::string&)> f);
template
int LibUser::writeAndReadAny(
  const std::string& name, const int& value, std::function<void(const int&)> f);
template
int LibUser::writeAndReadAny(
  const std::string& name, const double& value, std::function<void(const double&)> f);
template
int LibUser::writeAndReadAny(
  const std::string& name, const std::string& value, std::function<void(const std::string&)> f);

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

void LibUser::cb_int32(size_t pidx, int val)
{
  if (update_consumer_) {
    update_consumer_->UpdateInt32(pidx, val);
  }
}

void LibUser::cb_float64(size_t pidx, double val)
{
  if (update_consumer_) {
    update_consumer_->UpdateFloat64(pidx, val);
  }
}

void LibUser::cb_string(size_t pidx, const char* val)
{
  if (update_consumer_) {
    update_consumer_->UpdateString(pidx, val);
  }
}

void LibUser::cb_enum(size_t pidx, int val)
{
  if (update_consumer_) {
    update_consumer_->UpdateInt32(pidx, val);
  }
}

void LibUser::static_cb_int32(void* priv, size_t pidx, int val)
{
  reinterpret_cast<LibUser*>(priv)->cb_int32(pidx, val);
}

void LibUser::static_cb_float64(void* priv, size_t pidx, double val)
{
  reinterpret_cast<LibUser*>(priv)->cb_float64(pidx, val);
}

void LibUser::static_cb_string(void* priv, size_t pidx, const char* val)
{
  reinterpret_cast<LibUser*>(priv)->cb_string(pidx, val);
}

void LibUser::static_cb_enum(void* priv, size_t pidx, int val)
{
  reinterpret_cast<LibUser*>(priv)->cb_enum(pidx, val);
}

int LibUser::firstDriverParamIdx() const
{
  return first_driver_param_;
}

std::size_t LibUser::ap2lib(int asynport_param_idx) const
{
  return param_refs_.at(asynport_param_idx);
}

int LibUser::lib2ap(std::size_t libpidx) const
{
  try {
    return param_back_refs_.at(libpidx);
  } catch (const std::out_of_range&) {
    return -1;
  }
}

std::string LibUser::paramName(int asynport_param_idx) const
{
  try {
    return Lib::instance().params().at(ap2lib(asynport_param_idx)).drv_name;
  } catch (const std::out_of_range&) {
    return "UnknownParameter";
  }
}

const Param &LibUser::libParam(int asynport_param_idx) const
{
  return Lib::instance().params().at(ap2lib(asynport_param_idx));
}





