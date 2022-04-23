#pragma once

class iCreatedAtInit {
public:
  virtual int create(int dev_desc) = 0;
  virtual ~iCreatedAtInit() {}
};
