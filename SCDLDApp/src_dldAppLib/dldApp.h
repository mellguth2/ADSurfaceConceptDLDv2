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

/* ---------------   runtime interactions   ---------------------------- */
typedef void (*scdldapp_cb_int32)(void*, size_t, int);
typedef void (*scdldapp_cb_float64)(void*, size_t, double);
typedef void (*scdldapp_cb_string)(void*, size_t, const char*);
typedef void (*scdldapp_cb_enum)(void*, size_t, int);

/**
 * @brief create a user which is required in all other functions
 * @return a non-negative user_id if successful, else negative value
 */
int scdldapp_create_user();
/**
 * @brief delete a user
 * @param user_id as returned previously by scdldapp_create_user()
 */
void scdldapp_delete_user(int user_id);

/* all read/write functions return 0 on success, negative value on error */
int scdldapp_write_int32(int user_id, size_t pidx, int value);
int scdldapp_read_int32(int user_id, size_t pidx, int* value);
int scdldapp_write_float64(int user_id, size_t pidx, double value);
int scdldapp_read_float64(int user_id, size_t pidx, double* value);
int scdldapp_write_string(int user_id, size_t pidx, const char* value);

/**
 * @brief scdldapp_read_string read a string
 * @param pidx parameter index starting from 0
 * @param len if (*len) == 0, write required length into (*len) and don't touch
 * value; if (*len)>0, fill value with at most (*len) bytes
 * @param value buffer for receiving the string
 * @return 0 if successful, else negative
 */
int scdldapp_read_string(int user_id, size_t pidx, size_t* len, char* value);

int scdldapp_write_enum(int user_id, size_t pidx, int value);
int scdldapp_read_enum(int user_id, size_t pidx, int* value);

int scdldapp_set_callback_int32(int user_id, void* priv, scdldapp_cb_int32 cb);
int scdldapp_set_callback_float64(int user_id, void* priv, scdldapp_cb_float64 cb);
int scdldapp_set_callback_string(int user_id, void* priv, scdldapp_cb_string cb);
int scdldapp_set_callback_enum(int user_id, void* priv, scdldapp_cb_enum cb);
/* --------------------------------------------------------------------- */

#if 0
enum scdldapp_Datatype
{
  SC_DLDAPP_DATATYPE_INT32 = 0,
  SC_DLDAPP_DATATYPE_FLOAT64 = 1,
  SC_DLDAPP_DATATYPE_STRING = 2,
  SC_DLDAPP_DATATYPE_ENUM = 3
};

struct scdldapp_param_info1
{
  const char idname[29];
  const char description[29];
  int writable; // 1 if writable, 0 if read-only
  scdldapp_Datatype datatype;
};

struct scdldapp_param_int32_info1
{
  int initval;
  int minval;
  int maxval;
};

struct scdldapp_param_float64_info1
{
  double initval;
  double minval;
  double maxval;
};

/* ---------------   parameter meta         ---------------------------- */

/**
 * @brief get the number of parameters
 * @return number of parameters
 */
size_t scdldapp_get_nr_params();

/**
 * @brief get info about parameter selected by index
 * @param pidx parameter index starting from 0
 * @return 0 if successful, else negative
 */
int scdldapp_get_param_info1(size_t pidx, scdldapp_param_info1*);

/**
 * @brief get info about an int32 parameter
 * @param pidx parameter index starting from 0
 * @return 0 if successful, else negative
 */
int scdldapp_get_int32_info1(size_t pidx, scdldapp_param_int32_info1*);

/**
 * @brief get info about a float64 parameter
 * @param pidx parameter index starting from 0
 * @return 0 if successful, else negative
 */
int scdldapp_get_float64_info1(size_t pidx, scdldapp_param_float64_info1*);

/**
 * @brief get the number of choices of a enum parameter selected by index
 * @param pidx parameter index starting from 0
 * @return positive number of choices if sucessful, else negative
 */
int scdldapp_get_nr_choices(size_t pidx);

/**
 * @brief get the string of a seleted choice for an enum parameter
 * @param pidx parameter index starting from 0
 * @param cidx choice index starting from 0
 * @return choice string, or nullptr if error
 */
const char* scdldapp_get_choice_str(size_t pidx, size_t cidx);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus
