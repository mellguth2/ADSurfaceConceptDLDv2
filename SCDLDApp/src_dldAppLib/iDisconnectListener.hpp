#pragma once

/* Copyright 2022 Surface Concept GmbH */

class iDisconnectListener {
public:
  virtual void disconnect() = 0;
  virtual ~iDisconnectListener() {}
};
