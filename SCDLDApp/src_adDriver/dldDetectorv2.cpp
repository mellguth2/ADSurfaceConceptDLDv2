/* 
 * Copyright 2022 Surface Concept GmbH
 * dldDetectorv2.cpp
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsExit.h>
#include <cantProceed.h>
#include <iocsh.h>

#include "ADDriver.h"
#include <epicsExport.h>
#include "dldDetectorv2.h"
#include "dldApp.h"
#include "DldAppLibUser.hpp"
#include "ADUpdateConsumer.hpp"
#include "CachedArrays.hpp"
#include <sstream>
#include <iostream>

static const char *driverName = "dldDetectorv2";
static std::vector< std::unique_ptr<dldDetectorv2> > g_instances_;

/** Called when asyn clients call pasynInt32->write().
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus dldDetectorv2::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  const int param_idx = pasynUser->reason;

  auto handle_ret = [&](int ret) -> asynStatus {
    if (ret == 0) {
      return asynSuccess;
    }
    else if (ret != DLDAPPLIB_NOT_MY_PARAM) {
      std::ostringstream oss;
      oss << driverName << ":writeInt32() : app library returned error "
          << ret << " for parameter " << libusr_.paramName(param_idx)
          << " " << param_idx << std::endl;
      asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s\n", oss.str().c_str());
      throw -1;
      return asynError;
    }
    else {
      return static_cast<asynStatus>(PARAM_UNHANDLED); // (evil) ^^
    }
  };
  auto upPar = std::function<void(const int&)>(
    [&](const int& v) { setIntegerParam(param_idx, v); callParamCallbacks(); });

  asynStatus ret = handle_ret(libusr_.writeAndReadAny(param_idx, value, upPar));
  if (ret != static_cast<asynStatus>(PARAM_UNHANDLED)) return ret;

  // this point is reached if the application lib does not have any
  // corresponding parameter
  asynStatus status = setIntegerParam(param_idx, value);

  if (param_idx < libusr_.firstDriverParamIdx()) {
    status = ADDriver::writeInt32(pasynUser, value);
  }
  callParamCallbacks();
  if (status)
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
      "%s:writeInt32 error, status=%d function=%d, value=%d\n",
      driverName, status, param_idx, value);
  else
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
      "%s:writeInt32: function=%d, value=%d\n",
      driverName, param_idx, value);
  return status;
}


/** Called when asyn clients call pasynFloat64->write().
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus dldDetectorv2::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  const int param_idx = pasynUser->reason;
  auto handle_ret = [&](int ret) -> asynStatus {
    if (ret == 0) {
      return asynSuccess;
    }
    else if (ret != DLDAPPLIB_NOT_MY_PARAM) {
      std::ostringstream oss;
      oss << driverName << ":writeFloat64() : app library returned error "
          << ret << " for parameter " << libusr_.paramName(param_idx);
      asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s\n", oss.str().c_str());
      return asynError;
    }
    else {
      return static_cast<asynStatus>(PARAM_UNHANDLED);
    }
  };

  auto upPar = std::function<void(const double&)>(
    [&](const double& v) { setDoubleParam(param_idx, v); callParamCallbacks(); });

  asynStatus ret = handle_ret(libusr_.writeAndReadAny(param_idx, value, upPar));
  if (ret != static_cast<asynStatus>(PARAM_UNHANDLED)) return ret;

  // this point is reached if the application lib does not have any
  // corresponding parameter
  asynStatus status = setDoubleParam(param_idx, value);

  if (param_idx < libusr_.firstDriverParamIdx()) {
    status = ADDriver::writeFloat64(pasynUser, value);
  }

  callParamCallbacks();

  if (status)
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
            "%s:writeFloat64 error, status=%d function=%d, value=%f\n",
            driverName, status, param_idx, value);
  else
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
            "%s:writeFloat64: function=%d, value=%f\n",
            driverName, param_idx, value);
  return status;
}


/**
 * @brief Called when clients write a string (asynOctet) parameter
 */
asynStatus dldDetectorv2::writeOctet(
  asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual)
{
  // assume, that value is always null-terminated, inspite of getting nChars.
  // after all, setStringParam does not have a nChars argument
  const int param_idx = pasynUser->reason;
  auto handle_ret = [&](int ret) -> asynStatus {
    if (ret == 0) {
      return asynSuccess;
    }
    else if (ret != DLDAPPLIB_NOT_MY_PARAM) {
      std::ostringstream oss;
      oss << driverName << ":writeOctet() : app library returned error "
          << ret << " for parameter " << libusr_.paramName(param_idx);
      asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s\n", oss.str().c_str());
      return asynError;
    }
    else {
      return static_cast<asynStatus>(PARAM_UNHANDLED); // evil ^^
    }
  };

  auto upPar = std::function<void(const std::string&)>(
    [&](const std::string& v) {
      setStringParam(param_idx, v.c_str()); callParamCallbacks();
    });

  asynStatus ret =
    handle_ret(libusr_.writeAndReadAny(param_idx, std::string(value), upPar));
  if (ret != static_cast<asynStatus>(PARAM_UNHANDLED)) {
    *nActual = nChars; // TODO check if library imposes string length restrictions
    return ret;
  }
  // this point is reached if the parameter JSON specified an empty asynportname,
  // or the app library does not know the parameter.

  asynStatus status = setStringParam(param_idx, value);

  if (param_idx < libusr_.firstDriverParamIdx()) {
    status = ADDriver::writeOctet(pasynUser, value, nChars, nActual);
  }

  callParamCallbacks();

  if (status)
      asynPrint(pasynUser, ASYN_TRACE_ERROR,
            "%s:writeOctet error, status=%d function=%d, value=%s\n",
            driverName, status, param_idx, value);
  else
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
            "%s:writeOctet: function=%d, value=%s\n",
            driverName, param_idx, value);
  return status;
}

asynStatus dldDetectorv2::readInt8Array(
  asynUser* pasynUser, epicsInt8* value, size_t nElements, size_t* nIn)
{
  const int param_idx = pasynUser->reason;
  auto data_found = arrays_->getArray1D(
    param_idx,
    asynParamInt8Array,
    [this, value, nElements, nIn](void* data, std::size_t bytelen) {
      auto elements_to_copy = std::min(bytelen/sizeof(char), nElements);
      *nIn = elements_to_copy;
      memcpy(value, data, elements_to_copy * sizeof(char));
    }
  );
  if (data_found) {
    return asynSuccess;
  }
  *nIn = 0; // no data, return empty array
  return asynSuccess;
}

asynStatus dldDetectorv2::readInt16Array(
  asynUser* pasynUser, epicsInt16* value, size_t nElements, size_t* nIn)
{
  const int param_idx = pasynUser->reason;
  auto data_found = arrays_->getArray1D(
    param_idx,
    asynParamInt16Array,
    [this, value, nElements, nIn](void* data, std::size_t bytelen) {
      auto elements_to_copy = std::min(bytelen/sizeof(short), nElements);
      *nIn = elements_to_copy;
      memcpy(value, data, elements_to_copy * sizeof(short));
    }
  );
  if (data_found) {
    return asynSuccess;
  }
  *nIn = 0; // no data, return empty array
  return asynSuccess;
}

asynStatus dldDetectorv2::readInt32Array(
  asynUser* pasynUser, epicsInt32* value, size_t nElements, size_t* nIn)
{
  const int param_idx = pasynUser->reason;
  auto data_found = arrays_->getArray1D(
    param_idx,
    asynParamInt32Array,
    [this, value, nElements, nIn](void* data, std::size_t bytelen) {
      auto elements_to_copy = std::min(bytelen/sizeof(int), nElements);
      *nIn = elements_to_copy;
      memcpy(value, data, elements_to_copy * sizeof(int));
    }
  );
  if (data_found) {
    return asynSuccess;
  }
  *nIn = 0; // no data, return empty array
  return asynSuccess;
}

asynStatus dldDetectorv2::readFloat32Array(
  asynUser* pasynUser, epicsFloat32* value, size_t nElements, size_t* nIn)
{
  const int param_idx = pasynUser->reason;
  auto data_found = arrays_->getArray1D(
    param_idx,
    asynParamFloat32Array,
    [this, value, nElements, nIn](void* data, std::size_t bytelen) {
      auto elements_to_copy = std::min(bytelen/sizeof(float), nElements);
      *nIn = elements_to_copy;
      memcpy(value, data, elements_to_copy * sizeof(float));
    }
  );
  if (data_found) {
    return asynSuccess;
  }
  *nIn = 0; // no data, return empty array
  return asynSuccess;
}

asynStatus dldDetectorv2::readFloat64Array(
  asynUser* pasynUser, epicsFloat64* value, size_t nElements, size_t* nIn)
{
  const int param_idx = pasynUser->reason;
  auto data_found = arrays_->getArray1D(
    param_idx,
    asynParamFloat64Array,
    [this, value, nElements, nIn](void* data, std::size_t bytelen) {
      auto elements_to_copy = std::min(bytelen/sizeof(double), nElements);
      *nIn = elements_to_copy;
      memcpy(value, data, elements_to_copy * sizeof(double));
    }
  );
  if (data_found) {
    return asynSuccess;
  }
  *nIn = 0; // no data, return empty array
  return asynSuccess;
}


/** Report status of the driver.
  * Prints details about the driver if details>0.
  * It then calls the ADDriver::report() method.
  * \param[in] fp File pointed passed by caller where the output is written to.
  * \param[in] details If >0 then driver details are printed.
  */
void dldDetectorv2::report(FILE *fp, int details)
{

    fprintf(fp, "Surface Concept DLD %s\n", this->portName);
    if (details > 0) {
        int nx, ny, dataType;
        getIntegerParam(ADSizeX, &nx);
        getIntegerParam(ADSizeY, &ny);
        getIntegerParam(NDDataType, &dataType);
        fprintf(fp, "  NX, NY:            %d  %d\n", nx, ny);
        fprintf(fp, "  Data type:         %d\n", dataType);
    }
    /* Invoke the base class method */
    ADDriver::report(fp, details);
}

void dldDetectorv2::terminate()
{
  libusr_.resetUpdateConsumer();
  worker_.terminate();
}

/** Constructor for dldDetectorv2; most parameters are simply passed to ADDriver::ADDriver.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxSizeX The maximum X dimension of the images that this driver can create.
  * \param[in] maxSizeY The maximum Y dimension of the images that this driver can create.
  * \param[in] dataType The initial data type (NDDataType_t) of the images that this driver will create.
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */
dldDetectorv2::dldDetectorv2(
  const char *portName, int maxSizeX, int maxSizeY, NDDataType_t dataType,
  int maxBuffers, size_t maxMemory, int priority, int stackSize)
  : ADDriver(
      portName,
      DldApp::Lib::instance().numberArray2dParams(),
      DldApp::Lib::instance().numberDrvParams(),
      maxBuffers,
      maxMemory,
      asynFloat64ArrayMask | asynInt32ArrayMask,
      asynFloat64ArrayMask | asynInt32ArrayMask,
      0, 1, /* ASYN_CANBLOCK=0, ASYN_MULTIDEVICE=0, autoConnect=1 */
      priority,
      stackSize),
    pRaw(NULL)
{
  int status = asynSuccess;
  const char *functionName = "dldDetectorv2";

  // create all parameters that are defined by the library if they have an
  // entry in our database of EPICS records ("dldDetectorv2.template")
  lock();
  libusr_.createParams(
    [this](const char* name, asynParamType aptype, int* apidx) {
      return createParam(name, aptype, apidx);
    });
  // manually link parameters that are pre-defined by ADDriver base class
  // if they have a correspondence in the application library
  libusr_.linkParam(ADStatusMessage, asynParamOctet, "StatusMessage");
  libusr_.linkParam(ADStatus, asynParamInt32, "DetectorState");
  libusr_.linkParam(ADAcquire, asynParamInt32, "Acquire");
  libusr_.linkParam(ADImageMode, asynParamInt32, "ImageMode");
  libusr_.linkParam(ADNumImages, asynParamInt32, "NumImages");
  libusr_.linkParam(ADAcquireTime, asynParamFloat64, "Exposure");
  libusr_.linkParam(ADAcquirePeriod, asynParamFloat64, "AcquirePeriod");

  libusr_.linkParam(ADNumImagesCounter, asynParamInt32, "NumImagesCounter");
  libusr_.linkParam(ADBinX, asynParamInt32, "BinX");
  libusr_.linkParam(ADBinY, asynParamInt32, "BinY");
  libusr_.linkParam(ADMinX, asynParamInt32, "MinX");
  libusr_.linkParam(ADMinY, asynParamInt32, "MinY");
  libusr_.linkParam(ADSizeX, asynParamInt32, "SizeX");
  libusr_.linkParam(ADSizeY, asynParamInt32, "SizeY");


  std::unique_ptr<ADUpdateConsumer> upd_cons_{new ADUpdateConsumer(this)};
  arrays_ = &(upd_cons_->arrays());
  libusr_.setUpdateConsumer(std::move(upd_cons_));

  /* the following parameters are supplied by the ADDriver base
   * class */
  status |= setStringParam (ADManufacturer, "Surface Concept GmbH");
  status |= setStringParam (ADModel, "Delay Line Detector");
  status |= setIntegerParam(ADMaxSizeX, maxSizeX);
  status |= setIntegerParam(ADMaxSizeY, maxSizeY);
  status |= setIntegerParam(ADStatus, 9); // 9 = Disconnected
  status |= setIntegerParam(ADMinX, 0);
  status |= setIntegerParam(ADMinY, 0);
  status |= setIntegerParam(ADBinX, 1);
  status |= setIntegerParam(ADBinY, 1);
  status |= setIntegerParam(ADReverseX, 0);
  status |= setIntegerParam(ADReverseY, 0);
  status |= setIntegerParam(ADSizeX, maxSizeX);
  status |= setIntegerParam(ADSizeY, maxSizeY);
  status |= setIntegerParam(NDArraySizeX, maxSizeX);
  status |= setIntegerParam(NDArraySizeY, maxSizeY);
  status |= setIntegerParam(NDArraySize, 0);
  status |= setIntegerParam(NDDataType, dataType);
  status |= setIntegerParam(ADImageMode, ADImageContinuous);
  status |= setDoubleParam (ADAcquireTime, 0.50);
  status |= setDoubleParam (ADAcquirePeriod, .005);
  status |= setIntegerParam(ADNumImages, 10);

  unlock();

  if (status) {
    printf("%s: unable to set ADDriver / DLD parameters\n", functionName);
    return;
  }
}

/** Configuration command, called directly or from iocsh */
extern "C" int dldDetectorv2Config(
  const char *portName, int maxSizeX, int maxSizeY, int dataType,
  int maxBuffers, int maxMemory, int priority, int stackSize)
{
  g_instances_.emplace_back(
    new dldDetectorv2(portName, maxSizeX, maxSizeY, (NDDataType_t)dataType,
                      (maxBuffers < 0) ? 0 : maxBuffers,
                      (maxMemory < 0) ? 0 : maxMemory,
                      priority, stackSize));
  // dldDetectorv2 opens a WorkerThread which it must terminate when the IOC
  // exits -> register a notification via epicsAtExit.
  // Also, technically, when g_instances_ is deconstructed, all driver instances
  // are also deconstructed. If this leads to seg-faults because the IOC does
  // not stop calls into this driver early enough, replace unique_ptr
  // by plain C pointers in g_instances_ (and let OS do the clean-up, *sigh*)
  auto terminate = [](void* priv) {
    reinterpret_cast<dldDetectorv2*>(priv)->terminate();
  };
  epicsAtExit(terminate, g_instances_.back().get());
  return(asynSuccess);
}

/** Code for iocsh registration */
static const iocshArg cfgArg0 = {"Port name", iocshArgString};
static const iocshArg cfgArg1 = {"Max X size", iocshArgInt};
static const iocshArg cfgArg2 = {"Max Y size", iocshArgInt};
static const iocshArg cfgArg3 = {"Data type", iocshArgInt};
static const iocshArg cfgArg4 = {"maxBuffers", iocshArgInt};
static const iocshArg cfgArg5 = {"maxMemory", iocshArgInt};
static const iocshArg cfgArg6 = {"priority", iocshArgInt};
static const iocshArg cfgArg7 = {"stackSize", iocshArgInt};
static const iocshArg * const cfgArgs[] = {
  &cfgArg0,
  &cfgArg1,
  &cfgArg2,
  &cfgArg3,
  &cfgArg4,
  &cfgArg5,
  &cfgArg6,
  &cfgArg7 };
static const iocshFuncDef configdldDetectorv2 = {"dldDetectorv2Config", 8, cfgArgs};
static void configdldDetectorv2CallFunc(const iocshArgBuf *args)
{
  dldDetectorv2Config(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
    args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}

static void dldDetectorv2Register(void)
{
  iocshRegister(&configdldDetectorv2, configdldDetectorv2CallFunc);
}

extern "C" {
epicsExportRegistrar(dldDetectorv2Register);
}
