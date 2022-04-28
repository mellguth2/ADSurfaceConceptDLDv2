/* Copyright 2022 Surface Concept GmbH */
#include <epicsEvent.h>
#include "ADDriver.h"
#include <vector>
#include <unordered_map>
#include "WorkerThread.hpp"
#include "DldAppLibUser.hpp"

class CachedArrays;

/** Surface Concept GmbH, DLD detector driver version 2 */
class epicsShareClass dldDetectorv2 : public ADDriver {
  friend class ADUpdateConsumer;
public:
  static const int PARAM_UNHANDLED = 999;
  dldDetectorv2(const char *portName, int maxSizeX, int maxSizeY,
    NDDataType_t dataType,
    int maxBuffers, size_t maxMemory,
    int priority, int stackSize);

  void terminate();

  virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value) override;

  virtual asynStatus writeFloat64(
    asynUser *pasynUser, epicsFloat64 value) override;

  virtual asynStatus writeOctet(
    asynUser *pasynUser, const char *value, size_t nChars,
    size_t *nActual) override;

  virtual asynStatus readInt8Array(
    asynUser *pasynUser, epicsInt8 *value, size_t nElements, size_t *nIn)
    override;

  virtual asynStatus readInt16Array(
    asynUser *pasynUser, epicsInt16 *value, size_t nElements, size_t *nIn)
    override;

  virtual asynStatus readInt32Array(
    asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
    override;

  virtual asynStatus readFloat32Array(
    asynUser *pasynUser, epicsFloat32 *value, size_t nElements, size_t *nIn)
    override;

  virtual asynStatus readFloat64Array(
    asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
    override;

  virtual void report(FILE *fp, int details);

private:
  /* Our data */
  NDArray *pRaw;
  WorkerThread worker_;
  DldApp::LibUser libusr_;
  CachedArrays* arrays_;
};
