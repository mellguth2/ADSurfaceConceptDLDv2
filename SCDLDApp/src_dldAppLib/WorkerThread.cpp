/* Copyright 2022 Surface Concept GmbH */
#include "WorkerThread.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <iostream>
#include "sema.h"

struct WorkerThread::Priv {
  /* ---------------------------------------- */
  /*                data                      */
  /* ---------------------------------------- */
  enum MessageType {
    EMPTY_MESSAGE = 0,
    ADD_TASK = 1,
    TERMINATE = 2
  };
  struct Message {
    Message() : mtype(EMPTY_MESSAGE) {}
    Message(MessageType mt) : mtype(mt) {}
    Message(MessageType mt, std::function<void()> t) : mtype(mt), task(t) {}
    MessageType mtype;
    std::function<void()> task;
  };
  std::queue< Message > messages_;
  std::mutex messages_mutex_;
  std::unique_ptr<std::thread> thread_;
  Semaphore sema_message_;

  /* ---------------------------------------- */
  /*                functions                 */
  /* ---------------------------------------- */
  Priv() { thread_.reset(new std::thread( [this](){ job(); } )); }
  ~Priv() { terminate(); }
  bool alive() const { return thread_.operator bool(); }

  void addTask(std::function<void ()> t) {
    if (!alive()) return;
    {
      std::lock_guard<std::mutex> l(messages_mutex_);
      messages_.emplace(ADD_TASK, t);
    }
    sema_message_.signal();
  }

  void terminate() {
    if (!alive()) return;
    {
      std::lock_guard<std::mutex> l(messages_mutex_);
      messages_.emplace(TERMINATE);
    }
    sema_message_.signal();
    thread_->join();
    thread_.reset();
  }

  void job() {
    bool terminate = false;
    while (!terminate) {
      sema_message_.wait();
      Message entry;
      {
        std::lock_guard<std::mutex> l(messages_mutex_);
        if (messages_.empty()) continue;
        entry = messages_.front();
        messages_.pop();
      }
      switch (entry.mtype)
      {
      case EMPTY_MESSAGE:
        break;
      case ADD_TASK:
        try {
          entry.task();
        } catch (const std::exception& e) {
          std::cerr << "WorkerThread: Exception " << e.what() << std::endl;
        }
        break;
      case TERMINATE:
        terminate = true;
        break;
      }
    }
  }
};

WorkerThread::WorkerThread()
  : p_(new Priv)
{
}

WorkerThread::~WorkerThread()
{
  terminate();
}

void WorkerThread::addTask(std::function<void ()> task)
{
  p_->addTask(task);
}

void WorkerThread::terminate()
{
  p_->terminate();
}

bool WorkerThread::alive() const
{
  return p_->alive();
}
