#include "InitLibDataFromJSON.hpp"
#include <string>
#include "nlohmann/json.hpp"
#include "DldAppLibParams.hpp"
#include "dldApp.h"
#include <sstream>

namespace {
DldApp::DatatypeEnum libtypeFromString(const std::string& s) {
  static std::unordered_map<std::string, DldApp::DatatypeEnum> m;
  if (m.empty()) {
    m["enum"] = DldApp::DATATYPE_ENUM;
    m["int32"] = DldApp::DATATYPE_INT32;
    m["float64"] = DldApp::DATATYPE_FLOAT64;
    m["string"] = DldApp::DATATYPE_STRING;
    m["array1d"] = DldApp::DATATYPE_ARRAY1D;
    m["array2d"] = DldApp::DATATYPE_ARRAY2D;
  }
  try {
    return m.at(s);
  } catch (const std::out_of_range&)
  {
    return DldApp::DATATYPE_INVALID;
  }
}

DldApp::ElementDatatypeEnum elemtypeFromString(const std::string& s) {
  static std::unordered_map<std::string, DldApp::ElementDatatypeEnum> m;
  if (m.empty()) {
    m["u8"] = DldApp::ELEMTYPE_U8;
    m["i8"] = DldApp::ELEMTYPE_I8;
    m["u16"] = DldApp::ELEMTYPE_U16;
    m["i16"] = DldApp::ELEMTYPE_I16;
    m["u32"] = DldApp::ELEMTYPE_U32;
    m["i32"] = DldApp::ELEMTYPE_I32;
    m["u64"] = DldApp::ELEMTYPE_U64;
    m["i64"] = DldApp::ELEMTYPE_I64;
    m["f32"] = DldApp::ELEMTYPE_F32;
    m["f64"] = DldApp::ELEMTYPE_F64;
  }
  try {
    return m.at(s);
  } catch (const std::out_of_range&)
  {
    return DldApp::ELEMTYPE_INVALID;
  }
}

bool is_scalar_datatype (const std::string& s) {
  return (s.compare("enum")==0 || s.compare("int32")==0
      || s.compare("float64")==0 || s.compare("string")==0);
}
bool is_array(const std::string& s) {
  return (s.compare("array1d")==0 || s.compare("array2d")==0);
}
} // namespace

namespace DldApp {
int init_libdata_from_json(Lib& lib)
{
  try {
    auto j = nlohmann::json::parse(scdldapp_get_param_config_json());
    int nr_driver_params = 0; // <- count only the parameters where we
                              // create an asynPortDriver parameter, ourselves.
                              // This excludes parameters with an empty
                              // asynportname value in the JSON config
    for (std::size_t pidx = 0; pidx < j.size(); pidx++) {
      auto jpar = j.at(pidx);
      std::string node{jpar.at("node")};
      if (node.compare("parameter")!=0) {
        continue;
      }
      std::string libpname{jpar.at("name")};
      auto libptype = libtypeFromString(jpar.at("data type"));
      if (libptype == DldApp::DATATYPE_INVALID) {
        std::ostringstream oss;
        oss << "init_libdata_from_json(): invalid data type '"
            << jpar.at("data type") << "'";
        throw std::runtime_error(oss.str());
      }
      std::string drvname{jpar.at("epicsprops").at("asynportname")};
      if (!drvname.empty()) {
        nr_driver_params++;
      }
      lib.params_.emplace_back(libptype, drvname);
      // -> parameter added in vector
      // store additional meta data if parameter type is an array
      if(libptype == DATATYPE_ARRAY1D || libptype == DATATYPE_ARRAY2D) {
        std::size_t maxlen = jpar.at("maxlen");
        ElementDatatypeEnum etype =
          elemtypeFromString(jpar.at("element data type"));
        lib.params_.back().arr_cfg.reset(new ArrayParam(etype, maxlen));
      }
      lib.name2libidx_[libpname] = pidx;
    }
    lib.nr_driver_params_ = nr_driver_params;
  } catch (const nlohmann::json::exception&) {
    return INIT_LIBDATA_ERR_PARSE_JSON;
  }
  return 0;
}
}
