/* Copyright 2022 Surface Concept GmbH */
#include "TimeBin.hpp"
#include <scTDC.h>
#include <cmath>

int TimeBin::create(int dev_desc)
{
  int retcode = sc_tdc_get_binsize2(dev_desc, &binsize_);
  if (retcode < 0) {
    binsize_ = 1.0;
    return retcode;
  }
  return 0;
}

double TimeBin::operator()() const
{
  return binsize_;
}

TimeBin::Result TimeBin::autobins(
  double tstart_ns, double tsize_ns, unsigned long long size)
{
  if (tstart_ns < 0.0) tstart_ns = 0.0;
  if (tsize_ns < 0.01) tsize_ns = 0.01;
  if (tstart_ns > 1e9) tstart_ns = 1e9;
  if (tsize_ns > 1e9) tsize_ns = 1e9;
  Result r;
  r.size = size;
  r.binpow = 0;
  double b = binsize_;
  while (b <= 1e9) {
    r.tstart_ns = std::floor(tstart_ns / b) * b;
    auto size1 = std::floor(tsize_ns / b);
    r.tsize_ns = size1 * b;
    while (r.tstart_ns + r.tsize_ns < tstart_ns + tsize_ns && size1 < size) {
      size1 += 1.0;
      r.tsize_ns += b;
    }
    if (size1 < size) {
      r.offset = static_cast<unsigned long long>(tstart_ns / b);
      if (r.size < 1) {
        r.size = 1;
      }
      r.tsize_ns = r.size * b;
      break;
    }
    r.binpow++;
    b *= 2.0;
  }
  return r;
}

