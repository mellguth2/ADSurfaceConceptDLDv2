#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Surface Concept GmbH

INFILE="../../params/parameters.json"
OUTFILE="../_generated.template"
import json

def generate_template(infile, outfile):
  with open(infile, "r") as f_in:
    parameters = json.load(f_in)
    with open(outfile, "w") as f_out:
      # ----------
      #f_out.write(code1)
      # ----------
      pidx = 0
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        if not param['read-only']:
          pass
          #f_out.write(reg_wr[param['data type']](pidx, param['name']))
        #f_out.write(reg_rd[param['data type']](pidx, param['name']))
        pidx += 1
      # -----------
      #f_out.write(code2)
      # -----------
      pidx = 0
      for param in parameters:
        if param['node'] != 'parameter':
          continue
        #f_out.write(upd[param['data type']](pidx, param['name']))
        pidx += 1
      # -----------
      #f_out.write(code3)


if __name__ == "__main__":
  generate_template(INFILE, OUTFILE)
