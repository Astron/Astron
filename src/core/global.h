#pragma once
#include "../messagedirector/messagedirector.h"
#include "../dcparser/dcFile.h"
#include "logger.h"
#include "config.h"
#include <boost/asio.hpp>

Logger *gLogger = NULL;
ConfigFile *gConfig = NULL;
DCFile *gDCF = NULL;
boost::asio::io_service *io_service = NULL;