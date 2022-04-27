#ifndef SCTDC_HDF5_H
#define SCTDC_HDF5_H

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

/**
  @file
*/


#if defined _WIN32 || defined __CYGWIN__
	#ifdef SCTDC_HDF5_EXPORTS
		#define SCTDCHDF5DLL_PUBLIC __declspec(dllexport)
	#else
		#define SCTDCHDF5DLL_PUBLIC __declspec(dllimport)
	#endif
#elif __linux__
	#ifdef SCTDC_HDF5_EXPORTS
		#define SCTDCHDF5DLL_PUBLIC __attribute__ ((visibility("default")))
	#else
		#define SCTDCHDF5DLL_PUBLIC
	#endif
  #include <sys/types.h>
#else
  #error platform is not supported
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief creates a HDF5 streamer instance.
 * The HDF5 streamer instance is not tied to any TDC device after creation
 * (you need the connect function to do that).
 * @return non-negative id to be used as hdf5obj argument in other function
 * calls, or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_create();

/**
 * @brief destroys a HDF5 streamer instance.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_destroy(int hdf5obj);

/**
 * @brief connect the HDF5 streamer instance to an
 * initialized TDC device. This has no side effects. The streamer instance
 * just stores the device descriptor internally for later use when it is
 * activated.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @param dev_desc the TDC device descriptor as returned by sc_tdc_init_inifile
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_connect(int hdf5obj, int dev_desc);

/**
 * @brief disconnect the HDF5 streamer instance from a
 * TDC device. You may not need this function. It is useful to tell the streamer
 * instance not to try to close the USER_CALLBACKS pipe later if it is active
 * while the device is being disconnected.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_disconnect(int hdf5obj);

/**
 * @brief activate HDF5 streaming. Requires that
 * at least sc_tdc_hdf5_create, sc_tdc_hdf5_connect, sc_tdc_hdf5_cfg_outfile
 * have been called. If active > 0, this opens a USER_CALLBACKS pipe, then
 * opens the HDF5 file and will subsequently write any data on incoming DLD
 * events. If active == 0, the HDF5 file and the USER_CALLBACKS pipe will be
 * closed. A return value of 1 indicates that the streaming instance actually
 * switched to the active state. If activation fails, zero or a negative error
 * code may are returned. If deactivation succeeds, zero is returned.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @param active any value > 0 to activate, any value <= 0 to deactivate
 * @return 0 if not activated, 1 if activated or negative error code
 * (-10 ERR_INSTANCE_NOTEXIST, -3 FILE_ERROR)
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_setactive(int hdf5obj, int active);

/**
 * @brief query whether HDF5 streaming is active.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @return 0 if not active, 1 if active, or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_isactive(int hdf5obj);

/**
 * @brief set the file path of the output HDF5 file.
 * This must be called before activating the streaming by sc_tdc_hdf5_setactive.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @param fpath the file path
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_cfg_outfile(int hdf5obj, const char* fpath);

/**
 * @brief set a user comment to be included in the HDF5
 * file. This must be called before activating the streaming by
 * sc_tdc_hdf5_setactive.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @param msg the text of the user comment
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_cfg_comment(int hdf5obj, const char* msg);

/**
 * @brief select the event data fields to be written.
 * The mask parameter is a bitmask with the bits representing the following
 * event data fields (0xNN denotes a hexadecimal number) :
 * 0x1 start counter, 0x2 time tag, 0x4 subdevice, 0x8 channel,
 * 0x10 time since start pulse ("sum"), 0x20 "x" detector coordinate ("dif1"),
 * 0x40 "y" detector coordinate ("dif2"), 0x80 master reset counter,
 * 0x100 ADC value, 0x200 signal bit. If this function is not called, the
 * default will be that no data fields are selected, which will be of limited
 * use. The HDF5 file will then only receive millisecond markers and
 * start-of-measurement markers.
 * @param hdf5obj the object handle as returned by sc_tdc_hdf5_create
 * @param mask a bitmask of selected data fields
 * @return 0 on success or negative error code
 */
SCTDCHDF5DLL_PUBLIC int sc_tdc_hdf5_cfg_datasel(int hdf5obj, unsigned mask);

/**
 * @brief retrieve version string
 * @param buf user-provided buffer where the version string is copied to
 * @param len the length of the user-provided buffer
 */
SCTDCHDF5DLL_PUBLIC void sc_tdc_hdf5_version(char* buf, size_t len);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif
