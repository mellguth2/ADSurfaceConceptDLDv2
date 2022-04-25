TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== APPLICATION LOGIC LIBRARY ==============
# TODO: LIBRARY_IOC may not be the right thing(?). This is not the top-level
# support library for an IOC, but rather a dependency of the actual support
# library, which is libdldDetectorv2.so

INC += dldApp.h

LIBRARY_IOC = dldApp
LIB_SRCS += dldApp.cpp \
  DLD.cpp \
  WorkerThread.cpp \
  PipeRatemeter.cpp \
  PipeImageXY.cpp \
  TimeBin.cpp
USR_CXXFLAGS += -std=c++17

LIB_SYS_LIBS += scTDC

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

