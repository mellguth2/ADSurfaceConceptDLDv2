This is an EPICS areaDetector driver for delay-line detectors from
the Surface Concept GmbH (55124 Mainz, Germany) accessed through the 
"scTDC" SDK (software development kit).

The application logic of the driver is put into a separate library, 
(libdldApp.so --- sources in src_dldAppLib), which exposes an interface
for reading / writing and subscribing to a list of parameters and which
has no dependencies on EPICS libraries at all.

The areaDetector driver (libdldDetectorv2.so --- sources in src_adDriver)
accesses this interface and forwards write and read requests to this 
application library and subscribes to updates, including image data from
the application library.

The application library depends on an add-on library (libsctdc_hdf5.so ---
sources in src_sctdc_hdf5_lib) to stream lists of delay-line detector events
into HDF5 files. This library was developed against version 1.10.1 of the HDF5
libraries and tries to link against this version, which requires some 
preparation, please see notes in the Makefile of the "src_sctdc_hdf5_lib"
directory.

Since the application library API works with parameter indices which are
meaningless to human developers, some tooling / code generators are involved
to safely map these to meaningful names.
The source used by these code generators is the file params/parameters.json.
The Makefile in the same directory lists the code generators that are
being run (currently 3).
This file contains a list of parameter specifications implemented by the
application library. The content of this .json file is linked into the 
application library so as to provide means for the areaDetector driver to query
this list at startup of the IOC and automatically generate the parameters of
the asynportDriver underlying the ADDriver. A JSON schema for this file is
contained in json_schema, allowing to verify correctness of the parameters.json.
A code generator is used to also generate the "Db/dldDetectorv2.template" which
contains the EPICS database records specific to this driver, as well as the
"Db/dldDetectorv2_settings.req" which contains the respective autosave entries.

Q & A
=====

Q: I want to add scalar parameters and/or 1D waveforms to the driver
A: Edit the params/parameters.json and add an entry for the parameter. Either
   study the existing entries to find a parameter which is already similar, or
   check the json_schema/param.json and json_schema/epicsprops.json for
   information how to write the parameter entry.
   If you want to implement a parameter that is already pre-defined by the 
   ADDriver framework, use an empty string "" for the epicsprops->asynportname
   property. You will need to add a "linkParam" line in the 
   "src_adDriver/dldDetectorv2.cpp" in that case. Ideally, this should be the
   only case where editing of code in the "src_adDriver" folder is necessary.
   If you want to implement a parameter that is not pre-defined, choose a name
   for epicsprops->asynportname. The name you choose is used both by the code 
   generator for the "Db/dldDetectorv2.template" and by routines of the 
   areaDetector driver that read the json to automatically create the 
   asynportDriver parameters, so you won't have to type this name by hand 
   anywhere else.
   After editing the params/parameters.json, run the "verify" script (may need
   some python package to be installed) and make corrections if the script 
   indicates errors.
   Afterwards, running the "Makefile" from this here directory should run all
   necessary code generators first. The "src_sctdc_hdf5_lib" is not dependent
   on any of this, so once this builds fine, the make command proceeds to the 
   "src_dldAppLib", where the "glue.hpp" file has been updated by a code
   generator. This file tries to call functions from the DLD.hpp/DLD.cpp and
   it may complain about missing functions after adding a new parameter. You
   have to add these manually. The idea is that code generators touch only files
   that you don't edit, so there are no worries about loss of work due to code
   generators messing up manually created code. This neccessitates the manual
   addition of read/write functions to the DLD.hpp/DLD.cpp. However, the 
   compiler tells you the names of the missing functions. The glue.hpp also
   provides update_PARAMNAME functions that can be called from the DLD class,
   to send new values of parameters at any point in time. For 1D arrays and
   images, the update_XYZ functions are the only ones generated -> 1D arrays and
   images can only be used for sending data to the user, not getting input from 
   the user.
   If you create a 1D array parameter, stick to the element types "i8", "i16",
   "i32", "f32", "f64". Others are not yet supported.
   After implementing the functionality in the DLD class and rebuilding with
   make run from this directory, the newly built application library + 
   the auto-updated "Db/dldDetectorv2.template" enable the areaDetector driver 
   to immediately respect the newly added parameters.

Q: I want to add an image parameter
A: Images need an entry in the start script (st.cmd in some iocBoot directory)
   that looks like this:
   dbLoadRecords("NDStdArrays.template", "P=$(PREFIX),R=LiveImageXY:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),TYPE=Int32,FTVL=LONG,NELEMENTS=12000000")
   You should have unique addresses in the "ADDR=XYZ" part.
   In the parameters.json, you specify the same address in the
   epicsprops->address property.
   The only supported voxel type is currently 32-bit signed integer.
   The code generator for the "src_dldAppLib/glue.hpp" adds an update_XYZ 
   function for the new image, which can be used from the DLD class to send
   image data to areaDetector driver.

Q: Can I remove parameters?
A: Yes. The compiler won't complain about extra functions in the DLD class,
   so you may want to pay some attention to removing write_XYZ / read_XYZ 
   functions of the removed parameters.

