#include "global.h"

Logger *g_logger = new Logger;
ConfigFile *g_config = new ConfigFile;
dclass::File *g_dcf = new dclass::File;
boost::asio::io_service io_service;
std::unordered_map<doid_t, Uberdog> g_uberdogs;
