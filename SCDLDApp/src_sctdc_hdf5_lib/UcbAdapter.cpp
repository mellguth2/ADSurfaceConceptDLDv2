/*
 * Copyright (C) 2020 Surface Concept GmbH
*/
#include "UcbAdapter.hpp"
#include <scTDC.h>

UcbAdapterBase::UcbAdapterBase(void *parent) : p_(parent), dd_(-1), pd_(-1) {}

int UcbAdapterBase::deinstall()
{
  if (dd_ < 0 || pd_ < 0) return -1;
  int result = sc_pipe_close2(dd_, pd_);
  if (result >= 0) {
    dd_ = -1;
    pd_ = -1;
  }
  return result;
}
