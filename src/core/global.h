#pragma once
#include "../messagedirector/messagedirector.h"
#include "../dcparser/dcFile.h"
#include "logger.h"
#include "config.h"
#include <boost/asio.hpp>

extern Logger *gLogger;
extern ConfigFile *gConfig;
extern DCFile *gDCF;
extern boost::asio::io_service *io_service;