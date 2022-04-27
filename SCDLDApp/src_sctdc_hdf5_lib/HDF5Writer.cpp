/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#include "HDF5Writer.hpp"
#include "HDF5WriterImpl.hpp"

HDF5Writer::HDF5Writer()
{
  p.reset(new HDF5WriterImpl);
}

HDF5Writer::~HDF5Writer()
{
}

bool HDF5Writer::setActive(bool active)
{
  return p->setActive(active);
}

bool HDF5Writer::active() const
{
  return p->active();
}

void HDF5Writer::setConfig(const HDF5Config &c)
{
  p->setConfig(c);
}

const HDF5Config& HDF5Writer::config() const
{
  return p->config();
}

bool HDF5Writer::fileError() const
{
  return p->fileError();
}

void HDF5Writer::devConnected(int devdesc)
{
  p->devConnected(devdesc);
}

void HDF5Writer::devDisconnected()
{
  p->devDisconnected();
}

int HDF5Writer::deviceDescriptor() const
{
  return p->deviceDescriptor();
}
