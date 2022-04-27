#ifndef UCBADAPTER_HPP
#define UCBADAPTER_HPP

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

/*
 * UcpAdapter helps with using the USER_CALLBACKS pipe from a C++ class.
 * Use the UcbAdapter via the CRTP pattern, i.e. write a class X which
 * derives from UcbAdapter<X>. Then, add the following functions to this class:
 * void millisecond();
 * void start_of_meas();
 * void end_of_meas();
 * void statistics(const struct statistics_t *stat);
 * void tdc_event(const struct sc_TdcEvent *const event_array,
 *   size_t event_array_len);
 * void dld_event(const struct sc_DldEvent *const event_array,
 *   size_t event_array_len);
 * These functions will be called when the respective events occur.
 * Call install(dev_desc); to activate the callbacks
 * and deinstall(); to deactivate the callbacks.
 * No virtual functions are involved, so no vtable should be created by the
 * compiler (well, the static wrapper functions still have to be real
 * functions and they call your class functions, so, yes, there is this
 * indirection...)
 */

#include <scTDC.h>

/* part of UcbAdapter that is not dependent on template parameter */
struct UcbAdapterBase {
  void* p_;
  int dd_;
  int pd_;
  UcbAdapterBase(void* parent);
  int deinstall();
};

template <typename T>
struct UcbAdapter : public UcbAdapterBase
{
  UcbAdapter(T* parent) : UcbAdapterBase(parent) { }

  int install(int dev_desc) {
    if (dd_ > -1 || dev_desc < 0) return -1;
    dd_ = dev_desc;
    sc_pipe_callbacks pcb;
    pcb.priv = p_;
    pcb.dld_event = dld_event;
    pcb.tdc_event = tdc_event;
    pcb.start_of_measure = start_of_meas;
    pcb.end_of_measure = end_of_meas;
    pcb.statistics = statistics;
    pcb.millisecond_countup = millisecond;
    sc_pipe_callback_params_t pcbp;
    pcbp.callbacks = &pcb;
    int result = sc_pipe_open2(dd_, USER_CALLBACKS, &pcbp);
    if (result >= 0)
      pd_ = result;
    return result;
  }

  static void millisecond(void *priv) {
    reinterpret_cast<T*>(priv)->millisecond();
  }
  static void start_of_meas(void *priv) {
    reinterpret_cast<T*>(priv)->start_of_meas();
  }
  static void end_of_meas(void *priv) {
    reinterpret_cast<T*>(priv)->end_of_meas();
  }
  static void statistics(void *priv, const struct statistics_t *stat) {
    reinterpret_cast<T*>(priv)->statistics(stat);
  }
  static void tdc_event(void *priv,
    const struct sc_TdcEvent *const event_array,
    size_t event_array_len)
  {
    reinterpret_cast<T*>(priv)->tdc_event(event_array, event_array_len);
  }
  static void dld_event(void *priv,
    const struct sc_DldEvent *const event_array,
    size_t event_array_len)
  {
    reinterpret_cast<T*>(priv)->dld_event(event_array, event_array_len);
  }
};

#endif
