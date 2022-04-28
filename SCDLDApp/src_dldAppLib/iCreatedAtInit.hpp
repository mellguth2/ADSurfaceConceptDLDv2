#pragma once

/* Copyright 2022 Surface Concept GmbH */

class iCreatedAtInit {
public:
  virtual int create(int dev_desc) = 0;
  virtual ~iCreatedAtInit() {}
};
