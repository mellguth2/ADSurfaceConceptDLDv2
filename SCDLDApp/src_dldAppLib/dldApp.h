/* Copyright 2022 Surface Concept GmbH */

#pragma once

#if __linux__
#include "stddef.h"
#ifdef LIBDLDAPP_EXPORTS
#define LIBDLDAPP_PUBLIC __attribute__ ((visibility("default")))
#else
#define LIBDLDAPP_PUBLIC
#endif
#endif


#define SCDLDAPP_ERR_MAX_USERS -800000;
#define SCDLDAPP_ERR_NO_SUCH_USER -800001;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define SC_DLD_APP_LIB_VER_MAJ 0
#define SC_DLD_APP_LIB_VER_MIN 1
#define SC_DLD_APP_LIB_VER_PAT 0

/* ---------------   runtime interactions   ---------------------------- */
typedef void (*scdldapp_cb_int32)(void*, size_t, int);
typedef void (*scdldapp_cb_float64)(void*, size_t, double);
typedef void (*scdldapp_cb_string)(void*, size_t, const char*);
typedef void (*scdldapp_cb_enum)(void*, size_t, int);
typedef void (*scdldapp_cb_arr1d)(void*, size_t, size_t arr_len_in_bytes, void* data);
typedef void (*scdldapp_cb_arr2d)(void*, size_t, size_t arr_len_in_bytes,
                                  size_t width, void* data);

/**
 * @brief create a user which is required in all other functions
 * @return a non-negative user_id if successful, else negative value
 */
LIBDLDAPP_PUBLIC int scdldapp_create_user();
/**
 * @brief delete a user
 * @param user_id as returned previously by scdldapp_create_user()
 */
LIBDLDAPP_PUBLIC void scdldapp_delete_user(int user_id);

/* all read/write functions return 0 on success, negative value on error */
LIBDLDAPP_PUBLIC int scdldapp_write_int32(int user_id, size_t pidx, int value);
LIBDLDAPP_PUBLIC int scdldapp_read_int32(int user_id, size_t pidx, int* value);
LIBDLDAPP_PUBLIC int scdldapp_write_float64(int user_id, size_t pidx, double value);
LIBDLDAPP_PUBLIC int scdldapp_read_float64(int user_id, size_t pidx, double* value);
LIBDLDAPP_PUBLIC int scdldapp_write_string(int user_id, size_t pidx, const char* value);
/**
 * @brief scdldapp_read_string read a string
 * @param pidx parameter index starting from 0
 * @param len if (*len) == 0, write required length into (*len) and don't touch
 * value; if (*len)>0, fill value with at most (*len) bytes
 * @param value buffer for receiving the string
 * @return 0 if successful, else negative
 */
LIBDLDAPP_PUBLIC int scdldapp_read_string(int user_id, size_t pidx, size_t* len, char* value);
LIBDLDAPP_PUBLIC int scdldapp_write_enum(int user_id, size_t pidx, int value);
LIBDLDAPP_PUBLIC int scdldapp_read_enum(int user_id, size_t pidx, int* value);

LIBDLDAPP_PUBLIC int scdldapp_set_callback_int32(int user_id, void* priv, scdldapp_cb_int32 cb);
LIBDLDAPP_PUBLIC int scdldapp_set_callback_float64(int user_id, void* priv, scdldapp_cb_float64 cb);
LIBDLDAPP_PUBLIC int scdldapp_set_callback_string(int user_id, void* priv, scdldapp_cb_string cb);
LIBDLDAPP_PUBLIC int scdldapp_set_callback_enum(int user_id, void* priv, scdldapp_cb_enum cb);
LIBDLDAPP_PUBLIC int scdldapp_set_callback_arr1d(int user_id, void* priv, scdldapp_cb_arr1d cb);
LIBDLDAPP_PUBLIC int scdldapp_set_callback_arr2d(int user_id, void* priv, scdldapp_cb_arr2d cb);

LIBDLDAPP_PUBLIC const char* scdldapp_get_param_config_json();
LIBDLDAPP_PUBLIC void scdldapp_get_version(int* ver_maj, int* ver_min, int* ver_pat);

#ifdef __cplusplus
}
#endif // __cplusplus
