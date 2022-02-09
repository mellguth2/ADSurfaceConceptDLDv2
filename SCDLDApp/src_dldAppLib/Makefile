TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== APPLICATION LOGIC LIBRARY ==============

INC += dldApp.h

LIBRARY_IOC = dldApp
LIB_SRCS += dldApp.cpp DLD.cpp WorkerThread.cpp
USR_CXXFLAGS += -std=c++17

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

