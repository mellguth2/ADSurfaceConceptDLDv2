TOP = ..
include $(TOP)/configure/CONFIG

DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Db*))

DIRS += params src_sctdc_hdf5_lib src_dldAppLib src_adDriver

include $(TOP)/configure/RULES_DIRS

