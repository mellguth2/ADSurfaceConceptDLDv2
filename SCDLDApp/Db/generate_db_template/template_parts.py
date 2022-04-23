#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright 2022 Surface Concept GmbH
"""

TEMPLATE_START = \
"""
#=================================================================#
# Template file: dldDetectorv2.template
# Database for the records specific to the delay-line detector driver
#
# --- !!! CAUTION !!! -----------------------------------------------
# Do not edit this file. This file is generated by a script which needs
# to be re-run every time the application library is updated and the
# list of parameters exposed by this library has changed.
# -------------------------------------------------------------------
include "ADBase.template"

record(mbbo, "$(P)$(R)ColorMode")
{
   field(DISA, 1)
}

record(mbbi, "$(P)$(R)ColorMode_RBV")
{
   field(DISA, 1)
}
"""

def gen_desc_str(description):
  if description is not None:
    return '\n    field(DESC, "{}")'.format(description[:MAX_DESC_LEN])
  else:
    return ''

def gen_def_val_str(defaultval, out, type_cast):
  if defaultval is not None and out:
    return '\n    field(VAL, "{}")'.format(type_cast(defaultval))
  else:
    return ''

# ----------------------------------------------------------------------------
#                                   enum
# ----------------------------------------------------------------------------

MAX_DESC_LEN = 28
numberstrings = { 0:'ZR', 1:'ON', 2:'TW', 3:'TH', 4:'FR', 5:'FV', 6:'SX',
                  7:'SV', 8:'EI', 9:'NI', 10:'TE', 11:'EL', 12:'TV', 13:'TT',
                  14:'FT', 15:'FF' }
DB_GEN_ENUM_PREFIX_OUT = \
"""record(mbbo, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynInt32")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")<DEFVAL>
"""
DB_GEN_ENUM_PREFIX_IN = \
"""record(mbbi, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynInt32")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
"""
DB_GEN_ENUM_SUFFIX_OUT = \
"""    info(autosaveFields, "VAL")
}
"""
DB_GEN_ENUM_SUFFIX_IN = \
"""    field(SCAN, "I/O Intr")
}
"""
def db_gen_enum(name, asynportname, out=False, options={}, defaultval=None,
                description=None):
  prefix = DB_GEN_ENUM_PREFIX_OUT if out else DB_GEN_ENUM_PREFIX_IN
  if isinstance(defaultval, str):
    defaultval = options[defaultval]
  prefix = prefix.replace(
    '<NAME>',     name                                 ).replace(
    '<APNAME>',   asynportname                         ).replace(
    '<DESC>',     gen_desc_str(description)            ).replace(
    '<DEFVAL>',   gen_def_val_str(defaultval, out, int))
  opts = []
  for k, val in options.items():
    # k is the choice string, val is the choice integer (see options in .json)
    opts.append('    field({}VL, "{}")\n'.format(numberstrings[val], val))
    opts.append('    field({}ST, "{}")\n'.format(numberstrings[val], k))
  suffix = DB_GEN_ENUM_SUFFIX_OUT if out else DB_GEN_ENUM_SUFFIX_IN
  return prefix + "".join(opts) + suffix

# ----------------------------------------------------------------------------
#                                   int32
# ----------------------------------------------------------------------------

# TODO what about min/max ranges?
DB_GEN_INT32_OUT = \
"""record(longout, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynInt32")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")<DEFVAL>
    info(autosaveFields, "VAL")
}
"""

DB_GEN_INT32_IN = \
"""record(longin, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynInt32")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(SCAN, "I/O Intr")
}
"""

def db_gen_int32(name, asynportname, out=False, defaultval=None,
                 description=None):
  template = DB_GEN_INT32_OUT if out else DB_GEN_INT32_IN
  return template.replace(
    '<NAME>',     name                                 ).replace(
    '<APNAME>',   asynportname                         ).replace(
    '<DESC>',     gen_desc_str(description)            ).replace(
    '<DEFVAL>',   gen_def_val_str(defaultval, out, int))

# ----------------------------------------------------------------------------
#                                 float64
# ----------------------------------------------------------------------------

# TODO ai/ao records definitely have min/max range support
DB_GEN_FLOAT64_OUT = \
"""record(ao, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynFloat64")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(VAL,  "<DEFVAL>")
    field(PREC, "<PRECISION>")
    field(EGU, "<UNIT>")
    info(autosaveFields, "VAL")
}
"""

DB_GEN_FLOAT64_IN = \
"""record(ai, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynFloat64")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(VAL,  "<DEFVAL>")
    field(PREC, "<PRECISION>")
    field(EGU, "<UNIT>")
    field(SCAN, "I/O Intr")
}
"""

def db_gen_float64(name, asynportname, out=False, defaultval=0.0,
                   description=None, precision=0, unit=""):
  if isinstance(defaultval, str):
    defaultval = float(defaultval)
  def_val_str = "{:.Nf}".replace('N', str(precision)).format(defaultval)
  template = DB_GEN_FLOAT64_OUT if out else DB_GEN_FLOAT64_IN
  return template.replace(
    '<NAME>',      name                      ).replace(
    '<APNAME>',    asynportname              ).replace(
    '<DESC>',      gen_desc_str(description) ).replace(
    '<DEFVAL>',    def_val_str               ).replace(
    '<PRECISION>', str(precision)            ).replace(
    '<UNIT>',      unit                      )

# ----------------------------------------------------------------------------
#                                 string
# ----------------------------------------------------------------------------

DB_GEN_STRING_OUT = \
"""record(waveform, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynOctetWrite")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(FTVL, "CHAR")
    field(NELM, "<MAXLENGTH>")
    info(autosaveFields, "VAL")
}
"""

DB_GEN_STRING_IN = \
"""record(waveform, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynOctetRead")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(FTVL, "CHAR")
    field(NELM, "<MAXLENGTH>")
    field(SCAN, "I/O Intr")
}
"""

def db_gen_string(name, asynportname, out=False, defaultval=None,
                  description=None, maxlength=2048):
  template = DB_GEN_STRING_OUT if out else DB_GEN_STRING_IN
  return template.replace(
    '<NAME>',      name                      ).replace(
    '<APNAME>',    asynportname              ).replace(
    '<DESC>',      gen_desc_str(description) ).replace(
    '<MAXLENGTH>', str(maxlength)            )

# ----------------------------------------------------------------------------
#                                 array 1D
# ----------------------------------------------------------------------------

DB_GEN_ARRAY1D_IN = \
"""
record(waveform, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asyn<ELEMTYPE>ArrayIn")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(FTVL, "<FTVL>")
    field(NELM, "<MAXLENGTH>")
    field(SCAN, "I/O Intr")
}
"""

DB_ARRAY1D_ELEMTYPE_MAPPINGS = {
  'i8' :  ['Int8',    'CHAR'  ],
  'u8' :  ['Int16',   'SHORT' ], # needs conversion during updates
  'i16' : ['Int16',   'SHORT' ],
  'u16' : ['Int32',   'LONG'  ],  # needs conversion during updates
  'i32' : ['Int32',   'LONG'  ],
  'u32' : ['Float64', 'DOUBLE'],  # needs conversion during updates
  'i64' : ['Float64', 'DOUBLE'],  # needs conversion during updates
  'u64' : ['Float64', 'DOUBLE'],  # needs conversion during updates
  'f32' : ['Float32', 'FLOAT' ],
  'f64' : ['Float64', 'DOUBLE']
}

def db_gen_array1d(name, asynportname, out=False, defaultval=None,
                   description=None, maxlength=2048, elementtype='f64'):
  if out:
    raise RuntimeError("db_gen_array1d: parameter {}: writeable arrays are not "
                       "supported".format(name))
  asyn_elemtype = DB_ARRAY1D_ELEMTYPE_MAPPINGS[elementtype][0]
  ftvl = DB_ARRAY1D_ELEMTYPE_MAPPINGS[elementtype][1]
  return DB_GEN_ARRAY1D_IN.replace(
    '<NAME>',      name                      ).replace(
    '<APNAME>',    asynportname              ).replace(
    '<DESC>',      gen_desc_str(description) ).replace(
    '<MAXLENGTH>', str(maxlength)            ).replace(
    '<ELEMTYPE>',  asyn_elemtype             ).replace(
    '<FTVL>',      ftvl                      )