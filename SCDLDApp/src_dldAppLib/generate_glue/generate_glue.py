#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Surface Concept GmbH

INFILE="../../params/parameters.json"
OUTFILE="../glue.hpp"
import json
from glueparts import code1, code2, code3

# --- read/write functions, scalar types --------------------------------------
def reg_fun(action, datatype):
  s = '    <A>_<DT>_funs.insert({<PIDX>, &T::<A>_<NAME>});\n'.replace(
    '<A>', action).replace('<DT>', datatype)
  def reg1(pidx, name):
    return s.replace('<PIDX>', str(pidx)).replace('<NAME>', name)
  return reg1

reg_wr = { 'enum'   : reg_fun('write', 'enum'),
           'string' : reg_fun('write', 'string'),
           'int32'  : reg_fun('write', 'int'),
           'float64': reg_fun('write', 'float64') }
reg_rd = { 'enum'   : reg_fun('read', 'enum'),
           'string' : reg_fun('read', 'string'),
           'int32'  : reg_fun('read', 'int'),
           'float64': reg_fun('read', 'float64') }

# --- update functions, scalar types ------------------------------------------
def upd_fun(datatype, c_type):
  s = '  void update_<NAME>(<C_TYPE> v) { cb_<DT>.cb(cb_<DT>.priv, <PIDX>, v<MF>); }\n'
  s = s.replace('<DT>', datatype).replace('<C_TYPE>', c_type).replace(
    '<MF>', '.c_str()' if datatype == 'string' else '')
  def upd1(pidx, name):
    return s.replace('<PIDX>', str(pidx)).replace('<NAME>', name)
  return upd1

upd = { 'enum'  : upd_fun('enum', 'int'),
        'string': upd_fun('string', 'const std::string&'),
        'int32' : upd_fun('int32', 'int'),
        'float64' : upd_fun('float64', 'double') }

# --- update function 1d array ------------------------------------------------
element_data_type_to_ctype = {
  'i8'  : 'char',
  'u8'  : 'unsigned char',
  'i16' : 'short',
  'u16' : 'unsigned short',
  'i32' : 'int',
  'u32' : 'unsigned int',
  'i64' : 'long long',
  'u64' : 'unsigned long long',
  'f32' : 'float',
  'f64' : 'double'
}

def upd_fun_arr1d(elem_datatype):
  c_type = element_data_type_to_ctype[elem_datatype]
  s = '  void update_<NAME>(size_t nr_elem, <C_TYPE>* data) { \n    ' \
      'cb_arr1d.cb(cb_arr1d.priv, <PIDX>, nr_elem*sizeof(<C_TYPE>), data); }\n'
  s = s.replace('<C_TYPE>', c_type)
  def upd(pidx, name):
    return s.replace('<PIDX>', str(pidx)).replace('<NAME>', name)
  return upd

# --- update function 2d array ------------------------------------------------

# example
#  void update_LiveImageXY(size_t nr_elem, size_t width, int* data) {
#    cb_arr2d.cb(cb_arr2d.priv, 9, nr_elem*sizeof(int), width, data); }

def upd_fun_arr2d(elem_datatype):
  c_type = element_data_type_to_ctype[elem_datatype]
  s = '  void update_<NAME>(size_t nr_elem, size_t width, <C_TYPE>* data) {' \
      '\n    ' \
      'cb_arr2d.cb(cb_arr2d.priv, <PIDX>, nr_elem*sizeof(<C_TYPE>), width, data); }\n'
  s = s.replace('<C_TYPE>', c_type)
  def upd(pidx, name):
    return s.replace('<PIDX>', str(pidx)).replace('<NAME>', name)
  return upd

# -----------------------------------------------------------------------------
def generate_glue(infile, outfile):
  with open(infile, "r") as f_in:
    parameters = json.load(f_in)
    with open(outfile, "w") as f_out:
      # ----------
      f_out.write(code1)
      # ----------
      pidx = -1
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        pidx += 1
        if param['data type'] in ('array1d', 'array2d'):
          continue # no read/write funcs for arrays, yet.
        if not param['read-only']:
          f_out.write(reg_wr[param['data type']](pidx, param['name']))
        f_out.write(reg_rd[param['data type']](pidx, param['name']))
      # -----------
      f_out.write(code2)
      # -----------
      pidx = -1
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        pidx += 1
        if param['data type'] == 'array1d':
          f_out.write(upd_fun_arr1d(param['element data type'])(pidx, param['name']))
        elif param['data type'] == 'array2d':
          f_out.write(upd_fun_arr2d(param['element data type'])(pidx, param['name']))
        else:
          f_out.write(upd[param['data type']](pidx, param['name']))
      # -----------
      f_out.write(code3)


if __name__ == "__main__":
  generate_glue(INFILE, OUTFILE)
