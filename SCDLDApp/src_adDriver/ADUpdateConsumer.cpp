#include "ADUpdateConsumer.hpp"
#include "dldDetectorv2.h"
#include "DldAppLibUser.hpp"

// updates may be issued by the library synchronously in the course of
// processing a write-parameter-request or spontaneously at any point in
// time -> we need to lock, but we might already be locked, or not -> defer
// setting of parameters to a worker thread.

ADUpdateConsumer::ADUpdateConsumer(dldDetectorv2* parent) : parent_(parent) {
  arrays_.reset(new CachedArrays);
  /* // future support for images
    auto ins = [&](const std::string& s, int i) {
      to_ndarray[DldApp::Lib::instance().idxFromParamName(s)] = i;
    };
    ins("LiveImage", 0);
    */
}

void ADUpdateConsumer::UpdateInt32(std::size_t libpidx, int val) {
  int drvpidx = parent_->libusr_.lib2ap(libpidx);
  if (drvpidx >= 0) {
    parent_->worker_.addTask([this, drvpidx, val](){
      parent_->lock();
      parent_->setIntegerParam(drvpidx, val);
      parent_->callParamCallbacks();
      parent_->unlock();
    });
  }
}

void ADUpdateConsumer::UpdateFloat64(std::size_t libpidx, double val) {
  int drvpidx = parent_->libusr_.lib2ap(libpidx);
  if (drvpidx >= 0) {
    parent_->worker_.addTask([this, drvpidx, val](){
      parent_->lock();
      parent_->setDoubleParam(drvpidx, val);
      parent_->callParamCallbacks();
      parent_->unlock();
    });
  }
}

void ADUpdateConsumer::UpdateString(std::size_t libpidx, const std::string& val)
{
  int drvpidx = parent_->libusr_.lib2ap(libpidx);
  if (drvpidx >= 0) {
    parent_->worker_.addTask([this, drvpidx, val](){
      parent_->lock();
      parent_->setStringParam(drvpidx, val.c_str());
      parent_->callParamCallbacks();
      parent_->unlock();
    });
  }
}

void ADUpdateConsumer::UpdateArray1D(
  std::size_t libpidx, std::size_t bytelen, void* data)
{
  int drvpidx = parent_->libusr_.lib2ap(libpidx);
  if (drvpidx >= 0) {
    try {
      const auto& param = DldApp::Lib::instance().getParams().at(libpidx);
      DldApp::ElementDatatypeEnum elemtype = param.arr_cfg->elemtype;
      arrays_->updateArray1D(drvpidx, elemtype, bytelen, data);
      switch(elemtype) {
      case DldApp::ELEMTYPE_I32:
        parent_->worker_.addTask([this, drvpidx]() {
          parent_->lock();
          arrays_->getArray1D(
            drvpidx,
            asynParamInt32Array,
            [this, drvpidx] (void* data, std::size_t bytelen)
            {
              parent_->doCallbacksInt32Array(
                static_cast<epicsInt32*>(data), bytelen/sizeof(int), drvpidx, 0);
            });
          parent_->unlock();
        });
        break;
      case DldApp::ELEMTYPE_F32:
        parent_->worker_.addTask([this, drvpidx]() {
          parent_->lock();
          arrays_->getArray1D(
            drvpidx,
            asynParamFloat32Array,
            [this, drvpidx] (void* data, std::size_t bytelen)
            {
              parent_->doCallbacksFloat32Array(
                static_cast<epicsFloat32*>(data), bytelen/sizeof(epicsFloat32),
                drvpidx, 0);
            });
          parent_->unlock();
        });
        break;
      case DldApp::ELEMTYPE_F64:
        parent_->worker_.addTask([this, drvpidx]() {
          parent_->lock();
          arrays_->getArray1D(
            drvpidx,
            asynParamFloat64Array,
            [this, drvpidx] (void* data, std::size_t bytelen)
            {
              parent_->doCallbacksFloat64Array(
                static_cast<epicsFloat64*>(data), bytelen/sizeof(epicsFloat64),
                drvpidx, 0);
            });
          parent_->unlock();
        });
        break;
      default:
        break;
      }
    } catch (const std::out_of_range&) {}
  }
}

void ADUpdateConsumer::UpdateArray2D(
  std::size_t libpidx, std::size_t bytelen, std::size_t width, void* data)
{
  // TODO
}
