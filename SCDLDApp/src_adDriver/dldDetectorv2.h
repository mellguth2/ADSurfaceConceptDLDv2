#include <epicsEvent.h>
#include "ADDriver.h"
#include <vector>
#include <unordered_map>
#include "WorkerThread.hpp"
#include "DldAppLibParams.hpp"

/** Surface Concept GmbH, DLD detector driver version 2 */
class epicsShareClass dldDetectorv2 : public ADDriver {
public:
  dldDetectorv2(const char *portName, int maxSizeX, int maxSizeY,
    NDDataType_t dataType,
    int maxBuffers, size_t maxMemory,
    int priority, int stackSize);

  void terminate();

  /* These are the methods that we override from ADDriver */
  virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
  virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
  virtual void report(FILE *fp, int details);

private:
  /* Our data */
  NDArray *pRaw;
  WorkerThread worker_;
  DldAppLibUser app_lib_user_;
};
