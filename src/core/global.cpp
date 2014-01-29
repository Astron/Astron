#include "core/global.h"
#include "core/RoleFactory.h"
#include "config/ConfigGroup.h"

Logger *g_logger = new Logger;
ConfigFile *g_config = new ConfigFile;
DCFile *g_dcf = new DCFile;
boost::asio::io_service io_service;

// These values must be instantiated before they are used at runtime.
std::unordered_map<doid_t, Uberdog> g_uberdogs;

ConfigGroup ConfigGroup::root;
RoleFactory RoleFactory::singleton;
