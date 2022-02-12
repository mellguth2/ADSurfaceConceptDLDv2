#include "InitLibDataFromJSON.hpp"
#include <string>
#include "nlohmann/json.hpp"
#include "DldAppLibParams.hpp"
#include "dldApp.h"
#include <iostream>

namespace {
DldApp::DatatypeEnum libtypeFromString(const std::string& s) {
  static std::unordered_map<std::string, DldApp::DatatypeEnum> m;
  if (m.empty()) {
    m["enum"] = DldApp::DATATYPE_ENUM;
    m["int32"] = DldApp::DATATYPE_INT32;
    m["float64"] = DldApp::DATATYPE_FLOAT64;
    m["string"] = DldApp::DATATYPE_STRING;
  }
  try {
    return m.at(s);
  } catch (const std::out_of_range&)
  {
    std::cerr << "init_libdata_from_json(...): unsupported datatype " << s
              << std::endl;
    return DldApp::DATATYPE_INVALID;
  }
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
        continue;
      }
      std::string drvname{jpar.at("epicsprops").at("asynportname")};
      if (!drvname.empty()) {
        nr_driver_params++;
      }

      lib.params_.emplace_back(libptype, drvname);
      lib.name2libidx_[libpname] = pidx;
    }
    lib.nr_driver_params_ = nr_driver_params;
  } catch (const nlohmann::json::exception&) {
    return INIT_LIBDATA_ERR_PARSE_JSON;
  }
  return 0;
}
}
