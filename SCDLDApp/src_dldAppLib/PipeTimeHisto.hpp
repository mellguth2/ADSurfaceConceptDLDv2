#ifndef PIPETIMEHISTO_HPP
#define PIPETIMEHISTO_HPP

#include "iCreatedAtInit.hpp"
#include "iStartOfMeasListener.hpp"
#include "iEndOfMeasListener.hpp"
#include <functional>
#include <memory>

struct sc_pipe_dld_sum_histo_params_t;
class TimeBin;

class PipeTimeHisto
  : public iCreatedAtInit,
    public iStartOfMeasListener,
    public iEndOfMeasListener
{
public:
  // data_consumer_t args are nr_elements, xaxis values, yaxis values
  typedef std::function<void(size_t, double*, double*)> data_consumer_t;
  PipeTimeHisto(TimeBin&);
  virtual ~PipeTimeHisto();
  virtual int create(int dev_desc);
  virtual void start_of_measurement(int time_ms);
  virtual void end_of_measurement();
  void setDataConsumer(data_consumer_t);
  void setSizeT(int);
  void setMinTSI(double);
  void setSizeTSI(double);
  int sizeT() const;
  double minTSI() const;
  double sizeTSI() const;
  void setAccumulate(int);
  int accumulate() const;
private:
  static int static_allocator_cb(void* priv, void** buf);
  int allocator_cb(void** buf);
  void resize_data();
  void check_update_pipe();

  TimeBin& time_bin_;
  int dev_desc_ = -1;
  int pipe_desc_ = -1;
  bool change_request_ = false;
  data_consumer_t data_consumer_;
  std::unique_ptr<sc_pipe_dld_sum_histo_params_t> params_;
  std::unique_ptr<sc_pipe_dld_sum_histo_params_t> next_params_;
  std::vector<unsigned> data_;
  std::vector<double> xaxis_;
  std::vector<double> yaxis_;
  double user_tstart_ns_;
  double user_tsize_ns_;
  double actual_tstart_ns_;
  double actual_tsize_ns_;
  bool accumulate_;
};

#endif // PIPETIMEHISTO_HPP
