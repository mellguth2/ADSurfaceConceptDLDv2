/*
 * Copyright (C) 2020 Surface Concept GmbH
*/
#include "HDF5EventBuf.hpp"
#include <thread>
#include <chrono>
//#include <iostream>

void HDF5EventBuf::release_partial_page()
{
  activebuf_len_ = 0;
  _reset_buffer_ptrs();
}

void HDF5EventBuf::_reset_buffer_ptrs()
{
  for (unsigned i = 0; i < NR_OF_BUFS; i++) {
    if (cfg_.datasel.value & mask_from_bufid[i]) {
      buf_el_ptrs_[i] = buffers_[i+(activebuf_ * NR_OF_BUFS)];
    }
    else {
      buf_el_ptrs_[i] = &datadummy_;
    }
  }
}

void HDF5EventBuf::switchbuf(bool init)
{
  // update activebuf_, buf0full_, buf1full_
   if ((activebuf_ == 0) && !init) {
     buf0full_.store(true);
     while (buf1full_.load()) // have to wait until hdf5 thread clears the buf
       std::this_thread::sleep_for(std::chrono::milliseconds(1));
     //if (buf1full_.load()) throw HDF5EventBufFull();
     activebuf_ = 1;
   }
   else {
     buf1full_.store(!init);
     while (buf0full_.load()) // have to wait until hdf5 thread clears the buf
       std::this_thread::sleep_for(std::chrono::milliseconds(1));
     //if (buf0full_.load()) throw HDF5EventBufFull();
     activebuf_ = 0;
   }
   _reset_buffer_ptrs();
   activebuf_len_ = 0;
}


void HDF5EventBuf::initbufs()
{
  for (unsigned j = 0; j < 2; j++) {
    for (unsigned i = 0; i < NR_OF_BUFS; i++) {
      if (cfg_.datasel.value & mask_from_bufid[i]) {
        buffers_[i+(j*NR_OF_BUFS)] = malloc(cfg_.size*element_sizes[i]);
      }
      else {
        buffers_[i+(j*NR_OF_BUFS)] = nullptr;
        element_sizes[i] = 0;
      }
    }
  }
  switchbuf(true);
}

void HDF5EventBuf::deletebufs()
{
  for (unsigned j = 0; j < 2; j++)
    for (unsigned i = 0; i < NR_OF_BUFS; i++)
      if (cfg_.datasel.value & mask_from_bufid[i])
        free(buffers_[i+(j*NR_OF_BUFS)]);
}

template <typename T>
void HDF5EventBuf::fill_h5type(unsigned buf_id, const T& val) {
  // the DataType objects have to be kept, otherwise the IDs in element_h5types
  // expire
  h5type_objs[buf_id] = GetH5DataType(val);
  element_h5types[buf_id] = h5type_objs[buf_id].getId();
}


void HDF5EventBuf::fill_h5types()
{
  fill_h5type(BSTARTCTR, decltype(sc_DldEvent::start_counter)());
  fill_h5type(BTIMETAG, decltype(sc_DldEvent::time_tag)());
  fill_h5type(BSUBDEV, decltype(sc_DldEvent::subdevice)());
  fill_h5type(BCHANNEL, decltype(sc_DldEvent::channel)());
  fill_h5type(BSUM, decltype(sc_DldEvent::sum)());
  fill_h5type(BDIF1, decltype(sc_DldEvent::dif1)());
  fill_h5type(BDIF2, decltype(sc_DldEvent::dif2)());
  fill_h5type(BMRC, decltype(sc_DldEvent::master_rst_counter)());
  fill_h5type(BADC, decltype(sc_DldEvent::adc)());
  fill_h5type(BSIGBIT, decltype(sc_DldEvent::signal1bit)());
//  for (std::size_t i = 0; i < NR_OF_BUFS; i++)
//    std::cout << "element_h5types[" << i
  //              << "] = " << element_h5types[i] << std::endl;
}

void HDF5EventBuf::fill_names()
{
  // dataset names in HDF5 file
  names_.push_back("StartCtr");
  names_.push_back("TimeTag");
  names_.push_back("Subdevice");
  names_.push_back("Channel");
  names_.push_back("t");
  names_.push_back("x");
  names_.push_back("y");
  names_.push_back("MasterResetCtr");
  names_.push_back("ADC");
  names_.push_back("SignalBit");
}

