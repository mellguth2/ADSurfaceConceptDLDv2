/* Copyright 2022 Surface Concept GmbH */

#pragma once

#include <functional>
#include <memory>

/**
 * @brief a worker thread for fire-and-forget tasks
 */
class WorkerThread
{
  struct Priv;
public:
  WorkerThread();
  ~WorkerThread();
  void addTask(std::function<void()>);
  void terminate();
  bool alive() const;
private:
  std::unique_ptr<Priv> p_;
};
