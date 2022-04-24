#pragma once

#include <memory>
#include <string>
#include <asynParamType.h>

namespace DldApp
{
enum DatatypeEnum {
  DATATYPE_INVALID = 0,
  DATATYPE_ENUM = 1,
  DATATYPE_INT32 = 2,
  DATATYPE_FLOAT64 = 3,
  DATATYPE_STRING = 4,
  DATATYPE_ARRAY1D = 5,
  DATATYPE_ARRAY2D = 6
};
enum ElementDatatypeEnum {
  ELEMTYPE_INVALID = 0,
  ELEMTYPE_U8 = 0x01,   // unsigned integer
  ELEMTYPE_I8 = 0x11,   // signed integer
  ELEMTYPE_U16 = 0x02,
  ELEMTYPE_I16 = 0x12,
  ELEMTYPE_U32 = 0x03,
  ELEMTYPE_I32 = 0x13,
  ELEMTYPE_U64 = 0x04,
  ELEMTYPE_I64 = 0x14,
  ELEMTYPE_F32 = 0x23,  // C float
  ELEMTYPE_F64 = 0x24   // C double
};
inline size_t elementSize(ElementDatatypeEnum e) {
  if (e == ELEMTYPE_INVALID)
    return 0;
  else
    return (1u << ((static_cast<unsigned>(e) & 0x7) - 1));
}

struct ArrayParam {
  ArrayParam(ElementDatatypeEnum e, std::size_t l)
    : elemtype(e), maxlength(l), address(-1) {}
  ElementDatatypeEnum elemtype;  // C type for the elements of the array
  std::size_t maxlength;
  int address; // only for 2D arrays
};

struct Param {
  DatatypeEnum lib_type;  // data type in application library
  std::string drv_name;   // parameter name in the INP/OUT fields of
                          // dldDetectorv2.template, empty string if not ours
  asynParamType drv_type = asynParamNotDefined; // for manual linking of parameters, only
  std::unique_ptr<ArrayParam> arr_cfg;
  Param(
    DatatypeEnum lib_type_arg,
    const std::string& drv_name_arg = "")
    : lib_type(lib_type_arg),
      drv_name(drv_name_arg)
  { }
};
} // namespace
