#ifndef SC_EPICS_LIBDLDAPP_H
#define SC_EPICS_LIBDLDAPP_H

#if __linux__
#include "stddef.h"
#ifdef LIBDLDAPP_EXPORTS
#define LIBDLDAPP_PUBLIC __attribute__ ((visibility("default")))
#else
#define LIBDLDAPP_PUBLIC
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

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

typedef void (*scdldapp_cb_int32)(size_t, int);
typedef void (*scdldapp_cb_float64)(size_t, double);
typedef void (*scdldapp_cb_string)(size_t, const char*);
typedef void (*scdldapp_cb_enum)(size_t, int);

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

/* ---------------   runtime interactions   ---------------------------- */
/* all read/write functions return 0 on success, negative value on error */
int scdldapp_write_int32(size_t pidx, int value);
int scdldapp_read_int32(size_t pidx, int* value);
int scdldapp_write_float64(size_t pidx, double value);
int scdldapp_read_float64(size_t pidx, double* value);
int scdldapp_write_string(size_t pidx, const char* value);

/**
 * @brief scdldapp_read_string read a string
 * @param pidx parameter index starting from 0
 * @param len if (*len) == 0, write required length into (*len) and don't touch
 * value; if (*len)>0, fill value with at most (*len) bytes
 * @param value buffer for receiving the string
 * @return 0 if successful, else negative
 */
int scdldapp_read_string(size_t pidx, size_t* len, char* value);

int scdldapp_write_enum(size_t pidx, int value);
int scdldapp_read_enum(size_t pidx, int* value);

int scdldapp_set_callback_int32(scdldapp_cb_int32 cb);
int scdldapp_set_callback_float64(scdldapp_cb_float64 cb);
int scdldapp_set_callback_string(scdldapp_cb_string cb);
int scdldapp_set_callback_enum(scdldapp_cb_enum cb);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
