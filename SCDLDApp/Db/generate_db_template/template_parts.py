#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright 2022 Surface Concept GmbH
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
"""
record(mbbo, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynInt32")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")<DEFVAL>
"""[1:]
DB_GEN_ENUM_PREFIX_IN = \
"""
record(mbbi, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynInt32")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
"""[1:]
DB_GEN_ENUM_SUFFIX_OUT = \
"""    info(autosaveFields, "VAL")
}"""
DB_GEN_ENUM_SUFFIX_IN = \
"""    field(SCAN, "I/O Intr")
}"""
def db_gen_enum(name, asynportname, out=False, options={}, defaultval=None,
                description=None):
  prefix = DB_GEN_ENUM_PREFIX_OUT if out else DB_GEN_ENUM_PREFIX_IN
  prefix = prefix.replace(
    '<NAME>',     name                                 ).replace(
    '<APNAME>',   asynportname                         ).replace(
    '<DESC>',     gen_desc_str(description)            ).replace(
    '<DEFVAL>',   gen_def_val_str(defaultval, out, int))
  opts = []
  for k, val in options.items():
    # k is the choice string, val is the choice integer (see options in .json)
    opts.append('   field({}VL, "{}")\n'.format(numberstrings[val], val))
    opts.append('   field({}ST, "{}")\n'.format(numberstrings[val], k))
  suffix = DB_GEN_ENUM_SUFFIX_OUT if out else DB_GEN_ENUM_SUFFIX_IN
  return prefix + "".join(opts) + suffix

# ----------------------------------------------------------------------------
#                                   int32
# ----------------------------------------------------------------------------

DB_GEN_INT32_OUT = \
"""record(longout, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynInt32")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")<DEFVAL>
    info(autosaveFields, "VAL")
}"""

DB_GEN_INT32_IN = \
"""record(longin, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynInt32")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(SCAN, "I/O Intr")
}"""

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

DB_GEN_FLOAT64_OUT = \
"""record(ao, "$(P)$(R)<NAME>")
{
    field(PINI, "YES")
    field(DTYP, "asynFloat64")<DESC>
    field(OUT,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(VAL,  "<DEFVAL>")
    field(PREC, <PRECISION>)
    field(EGU, "<UNIT>")
    info(autosaveFields, "VAL")
}"""

DB_GEN_FLOAT64_IN = \
"""record(ai, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynFloat64")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(VAL,  "<DEFVAL>")
    field(PREC, <PRECISION>)
    field(EGU, "<UNIT>")
    field(SCAN, "I/O Intr")
}"""

def db_gen_float64(name, asynportname, out=False, defaultval=0.0,
                   description=None, precision=0, unit=""):
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
}"""

DB_GEN_STRING_IN = \
"""record(waveform, "$(P)$(R)<NAME>")
{
    field(DTYP, "asynOctetRead")<DESC>
    field(INP,  "@asyn($(PORT),$(ADDR),$(TIMEOUT))<APNAME>")
    field(FTVL, "CHAR")
    field(NELM, "<MAXLENGTH>")
    field(SCAN, "I/O Intr")
}"""

def db_gen_string(name, asynportname, out=False, description=None,
                  maxlength=2048):
  template = DB_GEN_STRING_OUT if out else DB_GEN_STRING_IN
  return template.replace(
    '<NAME>',      name                      ).replace(
    '<APNAME>',    asynportname              ).replace(
    '<DESC>',      gen_desc_str(description) ).replace(
    '<MAXLENGTH>', str(maxlength)            )