#include "core/global.h"
#include "core/RoleFactory.h"
#include "config/ConfigGroup.h"

/* Global Variables */
const dclass::File *g_dcf = nullptr;
std::unique_ptr<Logger> g_logger(new Logger);
std::unique_ptr<ConfigFile> g_config(new ConfigFile);
EventSender g_eventsender;
std::unordered_map<doid_t, Uberdog> g_uberdogs;
std::thread::id g_main_thread_id;
std::shared_ptr<uvw::Loop> g_loop;
