#include <epicsEvent.h>
#include "ADDriver.h"
#include <vector>
#include <unordered_map>
#include "WorkerThread.hpp"
#include "DldAppLibParams.hpp"

/** Surface Concept GmbH, DLD detector driver version 2 */
class epicsShareClass dldDetectorv2 : public ADDriver {
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

  virtual void report(FILE *fp, int details);

private:
  /* Our data */
  NDArray *pRaw;
  WorkerThread worker_;
  DldApp::LibUser libusr_;
};
