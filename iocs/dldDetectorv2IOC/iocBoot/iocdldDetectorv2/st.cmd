# Must have loaded envPaths via st.cmd.linux or st.cmd.win32

errlogInit(20000)

dbLoadDatabase("$(EPICS_HOME)/install/dbd/dldDetectorv2App.dbd")
dldDetectorv2App_registerRecordDeviceDriver(pdbbase) 

# Prefix for all records
epicsEnvSet("PREFIX", "SCDLD:")
# The port name for the detector
epicsEnvSet("PORT",   "DET1")
# The queue size for all plugins
epicsEnvSet("QSIZE",  "20")
# The maximim image width; used for row profiles in the NDPluginStats plugin
epicsEnvSet("XSIZE",  "2000")
# The maximim image height; used for column profiles in the NDPluginStats plugin
epicsEnvSet("YSIZE",  "1500")
# The maximum number of time series points in the NDPluginStats plugin
epicsEnvSet("NCHANS", "2048")
# The maximum number of frames buffered in the NDPluginCircularBuff plugin
epicsEnvSet("CBUFFS", "500")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(EPICS_HOME)/install/db")

# asynSetMinTimerPeriod(0.001) # this causes an error

# The EPICS environment variable EPICS_CA_MAX_ARRAY_BYTES needs to be set to a value at least as large
# as the largest image that the standard arrays plugin will send.
# That vlaue is $(XSIZE) * $(YSIZE) * sizeof(FTVL data type) for the FTVL used when loading the NDStdArrays.template file.
# The variable can be set in the environment before running the IOC or it can be set here.
# It is often convenient to set it in the environment outside the IOC to the largest array any client 
# or server will need.  For example 10000000 (ten million) bytes may be enough.
# If it is set here then remember to also set it outside the IOC for any CA clients that need to access the waveform record.  
# Do not set EPICS_CA_MAX_ARRAY_BYTES to a value much larger than that required, because EPICS Channel Access actually
# allocates arrays of this size every time it needs a buffer larger than 16K.
# Uncomment the following line to set it in the IOC.
#epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "10000000")

# dldDetectorv2Config(const char *portName, int maxSizeX, int maxSizeY, int dataType,
#                   int maxBuffers, int maxMemory, int priority, int stackSize)
dldDetectorv2Config("$(PORT)", $(XSIZE), $(YSIZE), 1, 0, 0)
# To have the rate calculation use a non-zero smoothing factor use the following line
#dbLoadRecords("dldDetectorv2.template",     "P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1,RATE_SMOOTH=0.2")
dbLoadRecords("$(EPICS_HOME)/install/db/dldDetectorv2.template","P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")

# Create a standard arrays plugin, set it to get data from detector driver.
NDStdArraysConfigure("Image1", 3, 0, "$(PORT)", 0)

# This creates a waveform large enough for 2000x2000x3 (e.g. RGB color) arrays.
# This waveform allows transporting 32-bit images
dbLoadRecords("NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),TYPE=Int32,FTVL=LONG,NELEMENTS=12000000")


# Load all other plugins using commonPlugins.cmd
# < $(ADCORE)/iocBoot/commonPlugins.cmd
set_requestfile_path("$(EPICS_HOME)/install/req")


iocInit()

# save things every thirty seconds
create_monitor_set("auto_settings.req", 30, "P=$(PREFIX)")
