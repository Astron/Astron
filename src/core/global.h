#pragma once
#include "messagedirector/MessageDirector.h"
#include "dcparser/dcFile.h"
#include "logger.h"
#include "config.h"
#include <boost/asio.hpp>

extern Logger *g_logger;
extern ConfigFile *g_config;
extern DCFile *g_dcf;
extern boost::asio::io_service io_service;
