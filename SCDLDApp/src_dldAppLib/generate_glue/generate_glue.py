#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Surface Concept GmbH

INFILE="../../params/parameters.json"
OUTFILE="../glue.hpp"
import json
from glueparts import code1, code2, code3

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

def upd_fun(datatype, cpp_type):
  s = '  void update_<NAME>(<C_ARG> v) { cb_<DT>.cb(cb_<DT>.priv, <PIDX>, v<MF>); }\n'
  s = s.replace('<DT>', datatype).replace('<C_ARG>', cpp_type).replace(
    '<MF>', '.c_str()' if datatype == 'string' else '')
  def upd1(pidx, name):
    return s.replace('<PIDX>', str(pidx)).replace('<NAME>', name)
  return upd1

upd = { 'enum'  : upd_fun('enum', 'int'),
        'string': upd_fun('string', 'const std::string&'),
        'int32' : upd_fun('int32', 'int'),
        'float64' : upd_fun('float64', 'double') }

def generate_glue(infile, outfile):
  with open(infile, "r") as f_in:
    parameters = json.load(f_in)
    with open(outfile, "w") as f_out:
      # ----------
      f_out.write(code1)
      # ----------
      pidx = 0
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        if not param['read-only']:
          f_out.write(reg_wr[param['data type']](pidx, param['name']))
        f_out.write(reg_rd[param['data type']](pidx, param['name']))
        pidx += 1
      # -----------
      f_out.write(code2)
      # -----------
      pidx = 0
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        f_out.write(upd[param['data type']](pidx, param['name']))
        pidx += 1
      # -----------
      f_out.write(code3)


if __name__ == "__main__":
  generate_glue(INFILE, OUTFILE)
