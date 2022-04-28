#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Surface Concept GmbH

INFILE="../../params/parameters.json"
OUTFILE="../dldDetectorv2.template"
OUTFILE_REQ="../dldDetectorv2_settings.req"
import json
import os
import shutil
from template_parts import TEMPLATE_START, db_gen_enum, db_gen_int32, \
  db_gen_float64, db_gen_string, db_gen_array1d

db_gen_fdict = {
  'enum'    : db_gen_enum,
  'int32'   : db_gen_int32,
  'float64' : db_gen_float64,
  'string'  : db_gen_string,
  'array1d' : db_gen_array1d}

def generate_template(infile, outfile):
  with open(infile, "r") as f_in:
    parameters = json.load(f_in)
    temp_outfile = outfile+"_TMP"
    try:
      with open(OUTFILE_REQ, "w") as f_req:
        with open(temp_outfile, "w") as f_out:
          write = f_out.write
          # ----------
          write(TEMPLATE_START)
          # ----------
          for param in parameters:
            if param['node'] != 'parameter':
              continue
            asynportname = param['epicsprops']['asynportname']
            # don't generate db entry for params provided by ADDriver base
            # class, indicated in the config by empty asynportname
            if len(asynportname) == 0:
              continue
            datatype = param['data type']
            if datatype == 'array2d':
              continue
            # add entry for the request file
            try:
              if param['persistent']:
                f_req.write('$(P)$(R){}\n'.format(param['name']))
            except KeyError:
              pass
            # add entry for the db file:
            # step 1 : select function for db record string generation
            f = db_gen_fdict[datatype]
            # step 2 : collect arguments to this function in f_args and f_kwargs
            f_args = [param['name'], asynportname]
            f_kwargs = {'defaultval'  : param['default'],
                        'description' : param['description']}
            # add specialties of individual data types
            if datatype == 'enum':
              f_kwargs['options'] = param['options']
            elif datatype == 'int32':
              pass # no specialties so far
            elif datatype == 'float64':
              f_kwargs['precision'] = param['precision']
              f_kwargs['unit'] = param['unit']
            elif datatype == 'string':
              f_kwargs['maxlength'] = param['maxlen']
            elif datatype == 'array1d':
              f_kwargs['maxlength'] = param['maxlen']
              f_kwargs['elementtype'] = param['element data type']
            # step 3 : invoke db record string generation
            # create a write parameter and a read-back parameter if not read-only:
            rwlist = (False,) if param['read-only'] else (False, True)
            nameextlist = ('',) if param['read-only'] else ('_RBV', '')
            for outval, nameext in zip(rwlist, nameextlist):
              f_args[0] = param['name'] + nameext
              f_kwargs['out'] = outval
              write(f(*f_args, **f_kwargs))
        shutil.copy(temp_outfile, outfile)
        f_req.write('file "ADBase_settings.req", P=$(P), R=$(R)\n')
    finally:
      os.remove(temp_outfile)

if __name__ == "__main__":
  generate_template(INFILE, OUTFILE)
