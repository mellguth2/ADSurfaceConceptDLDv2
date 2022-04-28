#pragma once

/* Copyright 2022 Surface Concept GmbH */

class iStartOfMeasListener {
public:
  virtual void start_of_measurement(int time_ms) = 0;
  virtual ~iStartOfMeasListener() {}
};
