/*
 * The MIT License
 *
 * Copyright 2017 Surface Concept GmbH <info@surface-concept.de>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * HDF5Utilities.h
 *
 */
#ifndef HDF5UTILITIES_H_
#define HDF5UTILITIES_H_

#include <string>
#include <cstdint>
#include "H5Cpp.h"

template <typename T>
H5::DataType GetH5DataType(T data);

template <>
inline H5::DataType GetH5DataType<int8_t>(int8_t)
{
  return H5::PredType::NATIVE_INT8;
}

template <>
inline H5::DataType GetH5DataType<uint8_t>(uint8_t)
{
  return H5::PredType::NATIVE_UINT8;
}

template <>
inline H5::DataType GetH5DataType<int16_t>(int16_t)
{
  return H5::PredType::NATIVE_INT16;
}

template <>
inline H5::DataType GetH5DataType<uint16_t>(uint16_t)
{
  return H5::PredType::NATIVE_UINT16;
}

template <>
inline H5::DataType GetH5DataType<int32_t>(int32_t)
{
  return H5::PredType::NATIVE_INT32;
}

template <>
inline H5::DataType GetH5DataType<uint32_t>(uint32_t)
{
  return H5::PredType::NATIVE_UINT32;
}

template <>
inline H5::DataType GetH5DataType<int64_t>(int64_t)
{
  return H5::PredType::NATIVE_INT64;
}

template <>
inline H5::DataType GetH5DataType<uint64_t>(uint64_t)
{
  return H5::PredType::NATIVE_UINT64;
}

#ifdef __linux__
template <>
inline H5::DataType GetH5DataType<unsigned long long>(unsigned long long)
{
  return H5::PredType::NATIVE_UINT64;
}
#endif

template <>
inline H5::DataType GetH5DataType<float>(float)
{
  return H5::PredType::NATIVE_FLOAT;
}

template <>
inline H5::DataType GetH5DataType<double>(double)
{
  return H5::PredType::NATIVE_DOUBLE;
}

template <>
inline H5::DataType GetH5DataType<std::string>(std::string data)
{
  return H5::StrType(H5::PredType::C_S1, data.size());
}

template <typename T>
void AttrWrite(H5::Attribute attr, H5::DataType type, T data)
{
  attr.write(type, &data);
}

template <>
inline void AttrWrite<std::string>(H5::Attribute attr, H5::DataType type,
                                   std::string data)
{
  attr.write(type, data);
}

template <typename T, typename U>
void WriteHDF5Attribute(U loc, std::string name, T data)
{
  H5::DataType type  = GetH5DataType(data);
  H5::Attribute attr = loc.createAttribute(name, type, H5::DataSpace());
  AttrWrite(attr, type, data);
}

#endif
