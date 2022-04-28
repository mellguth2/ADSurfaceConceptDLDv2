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
  void setMinX(int);
  void setMinY(int);
  void setSizeX(int);
  void setSizeY(int);
  void setBinX(int);
  void setBinY(int);
  int minX() const;
  int minY() const;
  int sizeX() const;
  int sizeY() const;
  int binX() const;
  int binY() const;
  void setAccumulate(int);
  int accumulate() const;
private:
  static int static_allocator_cb(void* priv, void** buf);
  int allocator_cb(void** buf);
  void resize_data();
  static int log2_(unsigned);

  int dev_desc_ = -1;
  int pipe_desc_ = -1;
  bool change_request_ = false;
  bool accumulate_ = false;
  data_consumer_t data_consumer_;
  std::unique_ptr<sc_pipe_dld_image_xy_params_t> params_;
  std::unique_ptr<sc_pipe_dld_image_xy_params_t> next_params_;
  std::vector<unsigned> data_;
};

#endif // PIPEIMAGEXY_HPP
