#pragma once

class iEndOfMeasListener {
public:
  virtual void end_of_measurement() = 0;
  virtual ~iEndOfMeasListener() {}
};
