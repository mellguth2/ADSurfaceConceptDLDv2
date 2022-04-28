#pragma once

/* Copyright 2022 Surface Concept GmbH */

#include "iCreatedAtInit.hpp"

class TimeBin : public iCreatedAtInit
{
  double binsize_ = 1.0;
public:
  struct Result {
    int binpow; // binning as a power of 2
    unsigned long long offset;
    unsigned long long size;
    double tstart_ns;
    double tsize_ns;
  };
  int create(int dev_desc) override;
  double operator()() const;

  /**
   * @brief If user chooses a time interval in nanoseconds and the number of
   * discrete points of the time axis, find a binning parameter (only powers
   * of 2 allowed when using scTDC builtin histograms) such that the complete
   * interval [tstart_ns, tsize_ns] will be covered. It is useful to let the
   * user define the number of discrete points because these determine the
   * memory requirement of the histogram.
   * @param tstart_ns
   * @param tsize_ns
   * @param size number of time bins on the time axis
   * @return Result structure, that yields the histogram parameters for
   * binning, offset and size of the time axis as well as the actual interval
   * in nanoseconds that is covered
   */
  Result autobins(
    double tstart_ns, double tsize_ns, unsigned long long size);
};
