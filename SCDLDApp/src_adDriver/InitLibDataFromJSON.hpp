#pragma once

/* Copyright 2022 Surface Concept GmbH */

/* The function declared here is put into its own compilation unit,
 * separate from DldAppLibParams.cpp, because we only need the big
 * nlohmann:json header-only library here and everywhere you include
 * nlohmann/json.hpp takes longer to compile */

#define INIT_LIBDATA_ERR_PARSE_JSON -1

namespace DldApp {
  class Lib;
  int init_libdata_from_json(Lib&);
}
