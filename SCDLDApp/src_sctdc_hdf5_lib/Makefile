TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== APPLICATION LOGIC LIBRARY ==============
# TODO: LIBRARY_IOC may not be the right thing(?). This is not the top-level
# support library for an IOC, but rather a dependency of dldAppLib

INC += scTDC_hdf5.h scTDC_hdf5_error_codes.h

LIBRARY_IOC = sctdc_hdf5
LIB_SRCS += scTDC_hdf5.cpp \
  HDF5DataFile.cpp \
  HDF5EventBuf.cpp \
  HDF5Writer.cpp \
  HDF5WriterImpl.cpp \
  UcbAdapter.cpp

USR_INCLUDES += -I${EPICS_BASE}/../HDF5/1.10.1/include
USR_LDFLAGS  += -L${EPICS_BASE}/../HDF5/1.10.1/lib
LIB_SYS_LIBS += hdf510_hl_cpp hdf510_cpp hdf510_hl hdf510
### the add-on library "sctdc_hdf5" has been developed using HDF5 1.10 libs
### I didn't find a way to specify particular library versions in the
### LIB_SYS_LIBS variable, so I created symlinks with names that work.
### List of created symbolic links:
###  ln -s libhdf5.so.1.10.1 libhdf510.so
###  ln -s libhdf5_cpp.so.1.10.1 libhdf510_cpp.so
###  ln -s libhdf5_hl_cpp.so.1.10.1 libhdf510_hl_cpp.so
###  ln -s libhdf5_hl.so.1.10.1 libhdf510_hl.so

USR_CXXFLAGS += -std=c++11
LIB_SYS_LIBS += scTDC

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

