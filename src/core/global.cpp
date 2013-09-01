#include "global.h"

Logger *gLogger = new Logger;
ConfigFile *gConfig = new ConfigFile;
DCFile *gDCF = new DCFile;
boost::asio::io_service io_service;