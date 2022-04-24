#ifndef PIPEIMAGEXY_HPP
#define PIPEIMAGEXY_HPP

#include "iCreatedAtInit.hpp"
#include "iStartOfMeasListener.hpp"
#include "iEndOfMeasListener.hpp"
#include <functional>
#include <memory>

struct sc_pipe_dld_image_xy_params_t;

class PipeImageXY
  : public iCreatedAtInit,
    public iStartOfMeasListener,
    public iEndOfMeasListener
{
public:
  // data_consumer_t args are nr_elements, width of image, data
  typedef std::function<void(size_t, size_t, int*)> data_consumer_t;
  PipeImageXY();
  virtual ~PipeImageXY();
  virtual int create(int dev_desc);
  virtual void start_of_measurement(int time_ms);
  virtual void end_of_measurement();
  void setDataConsumer(data_consumer_t);
private:
  static int static_allocator_cb(void* priv, void** buf);
  int allocator_cb(void** buf);

  int dev_desc_ = -1;
  int pipe_desc_ = -1;
  data_consumer_t data_consumer_;
  std::unique_ptr<sc_pipe_dld_image_xy_params_t> params_;
  std::vector<unsigned> data_;
};

#endif // PIPEIMAGEXY_HPP
