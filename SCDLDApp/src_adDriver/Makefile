TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== DETECTOR LIBRARY ==============

# The following gets rid of the -fno-implicit-templates flag on vxWorks, 
# so we get automatic template instantiation.
# This is what we want for miscellaneous/asynPortDriver.cpp
ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
CODE_CXXFLAGS=
endif

# INC specifies only the public header files that should be installed
# (header file dependencies of the driver code which determine the source files
# to be recompiled are automatically detected and need not be listed in INC)

INC += dldDetectorv2.h

LIBRARY_IOC = dldDetectorv2
LIB_SRCS += dldDetectorv2.cpp DldAppLibParams.cpp WorkerThread.cpp

LIB_LIBS += dldApp

DBD += dldDetectorv2support.dbd

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

