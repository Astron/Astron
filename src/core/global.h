#pragma once
#include "../messagedirector/messagedirector.h"
#include "logger.h"
#include "config.h"
#include <dcFile.h>
#include <boost/asio.hpp>

MessageDirector *gMD = NULL;
Logger *gLogger = NULL;
ConfigFile *gConfig = NULL;
DCFile *gDCF = NULL;
boost::asio::io_service *io_service = NULL;