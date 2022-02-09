#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Surface Concept GmbH

INFILE="../../params/parameters.json"
OUTFILE="../_generated.template"
import json
from template_parts import TEMPLATE_START, db_gen_enum, db_gen_int32, \
  db_gen_float64, db_gen_string

db_gen_fdict = {
  'enum'    : db_gen_enum,
  'int32'   : db_gen_int32,
  'float64' : db_gen_float64,
  'string'  : db_gen_string }

def generate_template(infile, outfile):
  with open(infile, "r") as f_in:
    parameters = json.load(f_in)
    with open(outfile, "w") as f_out:
      write = f_out.write
      # ----------
      write(TEMPLATE_START)
      # ----------
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        datatype = param['data type']
        f = db_gen_fdict[datatype] # selects the db record string generator
        # first two positional arguments are common across all db_gen_xxx(...):
        f_args = [param['name'], param['epicsprops']['asynportname']]
        # common keyword arguments:
        f_kwargs = {'defaultval'  : param['default'],
                    'description' : param['description']}
        ### add specialties of individual data types
        if datatype == 'enum':
          f_kwargs['options'] = param['options']
        elif datatype == 'int32':
          pass # no specialties so far
        elif datatype == 'float64':
          f_kwargs['precision'] = param['precision']
          f_kwargs['unit'] = param['unit']
        elif datatype == 'string':
          f_kwargs['maxlength'] = param['maxlen']
        # create a write parameter and a read-back parameter if not read-only:
        rwlist = (False,) if param['read-only'] else (False, True)
        nameextlist = ('',) if param['read-only'] else ('_RBV', '')
        for outval, nameext in (rwlist, nameextlist):
          f_args[0] = param['name'] + nameextlist
          f_kwargs['out'] = outval
          write(f(*f_args, **f_kwargs))

if __name__ == "__main__":
  generate_template(INFILE, OUTFILE)
