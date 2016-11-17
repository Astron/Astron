#include "core/global.h"
#include "core/RoleFactory.h"
#include "config/ConfigGroup.h"

/* Global Variables */
const dclass::File *g_dcf = nullptr;
std::unique_ptr<Logger> g_logger(new Logger);
ConfigFile *g_config = new ConfigFile;
boost::asio::io_service io_service;
std::unordered_map<doid_t, Uberdog> g_uberdogs;
