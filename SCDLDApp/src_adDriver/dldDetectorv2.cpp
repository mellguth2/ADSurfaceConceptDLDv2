/* 
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
#include "DldAppLibParams.hpp"
#include <sstream>

static const char *driverName = "dldDetectorv2";
static std::vector< std::unique_ptr<dldDetectorv2> > g_instances_;

/** Called when asyn clients call pasynInt32->write().
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus dldDetectorv2::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int param_idx = pasynUser->reason;

  int ret = app_lib_user_.writeInt32(param_idx, value);
  if (ret == 0) {
    // no setIntegerParam(...) here! The lib issues updates for that.
    return asynSuccess;
  }
  else if (ret != DLDAPPLIB_NOT_MY_PARAM) {
    std::ostringstream oss;
    oss << driverName << ":writeInt32() : app library returned error "
        << ret << " for parameter " << app_lib_user_.paramName(param_idx);
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s\n", oss.str().c_str());
    return asynError;
  }
  // this point is reached if the app library does not know the parameter

  asynStatus status = setIntegerParam(param_idx, value);

  if (param_idx < app_lib_user_.firstDriverParamIdx()) {
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
  int param_idx = pasynUser->reason;
  int ret = app_lib_user_.writeFloat64(param_idx, value);
  if (ret == 0) {
    // no setDoubleParam(...) here! The lib issues updates for that.
    return asynSuccess;
  }
  else if (ret != DLDAPPLIB_NOT_MY_PARAM) {
    std::ostringstream oss;
    oss << driverName << ":writeFloat64() : app library returned error "
        << ret << " for parameter " << app_lib_user_.paramName(param_idx);
    asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s\n", oss.str().c_str());
    return asynError;
  }
  // this point is reached if the app library does not know the parameter

  asynStatus status = setDoubleParam(param_idx, value);

  if (param_idx < app_lib_user_.firstDriverParamIdx()) {
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


/** Report status of the driver.
  * Prints details about the driver if details>0.
  * It then calls the ADDriver::report() method.
  * \param[in] fp File pointed passed by caller where the output is written to.
  * \param[in] details If >0 then driver details are printed.
  */
void dldDetectorv2::report(FILE *fp, int details)
{

    fprintf(fp, "Simulation detector %s\n", this->portName);
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
      1, // max_instances
      DldAppLib::instance().numberParams(),
      maxBuffers,
      maxMemory,
      0, 0, /* No interfaces beyond those set in ADDriver.cpp */
      0, 1, /* ASYN_CANBLOCK=0, ASYN_MULTIDEVICE=0, autoConnect=1 */
      priority,
      stackSize),
    pRaw(NULL)
{
  int status = asynSuccess;
  const char *functionName = "dldDetectorv2";

  app_lib_user_.createParams(
    [this](const char* name, asynParamType aptype, int* apidx) {
      return createParam(name, aptype, apidx);
    });

  /* the following parameters are supplied by the ADDriver base
   * class */
  status |= setStringParam (ADManufacturer, "Surface Concept GmbH");
  status |= setStringParam (ADModel, "Delay Line Detector");
  status |= setIntegerParam(ADMaxSizeX, maxSizeX);
  status |= setIntegerParam(ADMaxSizeY, maxSizeY);
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
