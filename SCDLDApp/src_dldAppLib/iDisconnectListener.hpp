#pragma once

class iDisconnectListener {
public:
  virtual void disconnect() = 0;
  virtual ~iDisconnectListener() {}
};
