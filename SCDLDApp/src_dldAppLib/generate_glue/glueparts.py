#!/usr/bin/env python3
# -*- coding: utf-8 -*-

code1 = \
"""
#pragma once

/* This file is auto-generated by a script glue_generator.py.
 * Do not edit this file by hand.
 */

#include <stddef.h>
#include <unordered_map>
#include <functional>
#include <string>
#include <cstring>

#define GLUE_ERR_OUT_OF_RANGE -900001

template <typename T>
class Glue
{
  T* parent_;
  typedef void (*cb_int32_t)(void*, size_t, int);
  typedef void (*cb_float64_t)(void*, size_t, double);
  typedef void (*cb_string_t)(void*, size_t, const char*);
  typedef void (*cb_enum_t)(void*, size_t, int);
  template <typename CBType>
  struct RegCallback
  {
    RegCallback() : priv(nullptr), cb(nullptr) {}
    RegCallback(void* p, CBType c) : priv(p), cb(c) {}
    void set(void* p, CBType c) { priv = p; cb = c; }
    void* priv;
    CBType cb;
  };
  RegCallback<cb_int32_t> cb_int32;
  RegCallback<cb_float64_t> cb_float64;
  RegCallback<cb_string_t> cb_string;
  RegCallback<cb_enum_t> cb_enum;
  // define member function signatures for the T class
  typedef int (T::*write_int_member_fun_t) (int);
  typedef int (T::*write_float64_member_fun_t) (double);
  typedef int (T::*write_string_member_fun_t) (const std::string&);
  typedef int (T::*read_int_member_fun_t) (int*);
  typedef int (T::*read_float64_member_fun_t) (double*);
  typedef int (T::*read_string_member_fun_t) (std::string&);
  std::unordered_map<size_t, write_int_member_fun_t> write_int_funs;
  std::unordered_map<size_t, write_int_member_fun_t> write_enum_funs;
  std::unordered_map<size_t, write_float64_member_fun_t> write_float64_funs;
  std::unordered_map<size_t, write_string_member_fun_t> write_string_funs;
  std::unordered_map<size_t, read_int_member_fun_t> read_int_funs;
  std::unordered_map<size_t, read_int_member_fun_t> read_enum_funs;
  std::unordered_map<size_t, read_float64_member_fun_t> read_float64_funs;
  std::unordered_map<size_t, read_string_member_fun_t> read_string_funs;

public:
  Glue(T* parent) : parent_(parent) {
    cb_int32.cb = [](void*, size_t, int) { };
    cb_float64.cb = [](void*, size_t, float) { };
    cb_string.cb = [](void*, size_t, const char*) { };
    cb_enum.cb = [](void*, size_t, int) { };
    /* ---------- */
"""


code2 = \
"""  }

  int write_int(size_t pidx, int value) {
    try {
      return std::invoke(write_int_funs.at(pidx), parent_, value);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int write_float64(size_t pidx, double value) {
    try {
      return std::invoke(write_float64_funs[pidx], parent_, value);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int write_enum(size_t pidx, int value) {
    try {
      return std::invoke(write_enum_funs.at(pidx), parent_, value);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int write_string(size_t pidx, const char* value) {
    try {
      return std::invoke(write_string_funs[pidx], parent_, value);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int read_int(size_t pidx, int* dest) {
    try {
      return std::invoke(read_int_funs[pidx], parent_, dest);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int read_float64(size_t pidx, double* dest) {
    try {
      return std::invoke(read_float64_funs[pidx], parent_, dest);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int read_enum(size_t pidx, int* dest) {
    try {
      return std::invoke(read_enum_funs[pidx], parent_, dest);
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int read_string(size_t pidx, size_t* len, char* value) {
    try {
      std::string s;
      int ret = std::invoke(read_string_funs[pidx], parent_, s);
      if (ret < 0) return ret;
      if (len != nullptr && value == nullptr) {
        *len = s.size()+1;
      }
      else if (len != nullptr && value != nullptr) {
        strncpy(value, s.c_str(), (*len)-1);
        value[(*len)-1] = 0;
      }
      return ret;
    } catch (const std::out_of_range&) {
      return GLUE_ERR_OUT_OF_RANGE;
    }
  }
  int set_callback_int32(void* priv, cb_int32_t cb) { cb_int32.set(priv, cb); return 0; }
  int set_callback_float64(void* priv, cb_float64_t cb) { cb_float64.set(priv, cb); return 0; }
  int set_callback_string(void* priv, cb_string_t cb) { cb_string.set(priv, cb); return 0; }
  int set_callback_enum(void* priv, cb_enum_t cb) { cb_enum.set(priv, cb); return 0; }
"""

code3 = \
"""
};
"""
