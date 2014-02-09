#include "core/global.h"
#include "core/RoleFactory.h"
#include "config/ConfigGroup.h"

/* Global Variables */
Logger *g_logger = new Logger;
ConfigFile *g_config = new ConfigFile;
DCFile *g_dcf = new DCFile;
boost::asio::io_service io_service;
std::unordered_map<doid_t, Uberdog> g_uberdogs;
