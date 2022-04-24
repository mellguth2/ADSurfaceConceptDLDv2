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
  int addr = parent_->libusr_.array2d_address(libpidx);
  auto elemtype = parent_->libusr_.element_type(libpidx);
  auto maxlength = parent_->libusr_.array_maxlength(libpidx);
  if (addr < 0 || addr >= DldApp::Lib::instance().numberArray2dParams()
      || elemtype == DldApp::ELEMTYPE_INVALID
      || width == 0)
  {
    return;
  }
  //
  // (1) we have no guarantees from the libdldApp that the data memory buffer will
  // live longer than when we return from this function, so we make a copy here
  // (by calling arrays_->updateImage)
  // (2) ADUpdateConsumer::UpdateArray2D(...) may be called at any time, so we
  // don't know whether we can lock the ADDriver and we might not want to block
  // the app library, so we defer ADDriver actions to a separate worker thread.
  // Currently, these circumstances lead us to making 2 copies of the image,
  // one in arrays_->updateImage(...), and one memcpy down below in the worker
  // task. This is a bit wasteful, but at least safe.
  // We could pass our copied data buffer to the pNDArrayPool->alloc, thereby
  // eliminating the 2nd copy, but our copied data buffer does not necessarily
  // remain unchanged until all users of the NDArray have released it.
  arrays_->updateImage(addr, elemtype, maxlength, bytelen, width, data);
  parent_->worker_.addTask([this, addr, bytelen]() {
    parent_->lock();
    bool image_found = arrays_->getImage(
      addr,
      [this, addr, bytelen](void* data, std::size_t width, std::size_t height) {
        auto& pArr = parent_->pArrays[addr];
        if (pArr != 0) {
          pArr->release();
        }
        size_t dims[] = {height, width};
        pArr = parent_->pNDArrayPool->alloc(2, dims, NDInt32, 0, NULL);
        if (!pArr) {
          return;
        }
        parent_->updateTimeStamp(&(pArr->epicsTS));
        NDArrayInfo_t info;
        pArr->getInfo(&info);
        memcpy(pArr->pData, data, std::min(bytelen, info.totalBytes));
      });
    parent_->unlock();
    if (image_found) {
      auto& pArr = parent_->pArrays[addr];
      if (pArr != nullptr) {
        parent_->doCallbacksGenericPointer(pArr, parent_->NDArrayData, addr);
      }
    }
  });
}
