#include "global.h"

Logger *g_logger = new Logger;
ConfigFile *g_config = new ConfigFile;
DCFile *g_dcf = new DCFile;
boost::asio::io_service io_service;
