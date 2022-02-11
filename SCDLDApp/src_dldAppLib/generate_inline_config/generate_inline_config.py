#!/usr/bin/env python3

""" This script generates C code for a const char* string literal 
mirroring the contents of the INFILE """

OUTFILE="../dldApp_inline_config.hpp"
INFILE="../../params/parameters.json"

HEADER= \
"""// Do not edit this file. Edit and run generate_inline_config.py, instead.
// This file is to be included in exactly one .cpp file.

const char* param_config_data = """

if __name__ == "__main__":
  with open(INFILE, "r") as f_in:
    with open(OUTFILE, "w") as f:
      f.write(HEADER)
      for l in f_in:
        while l.endswith('\n') or l.endswith('\r'):
          l = l[:-1]
        f.write('\n')
        l = l.replace('"', '\\"') # escape quotation marks
        f.write('  "' + l + '\\n"')
      f.write(';\n')
