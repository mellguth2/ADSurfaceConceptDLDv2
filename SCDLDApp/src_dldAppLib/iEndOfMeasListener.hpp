#pragma once

/* Copyright 2022 Surface Concept GmbH */

class iEndOfMeasListener {
public:
  virtual void end_of_measurement() = 0;
  virtual ~iEndOfMeasListener() {}
};
