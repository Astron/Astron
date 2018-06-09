#pragma once
#include "Logger.h"
#include "config/ConfigVariable.h"
#include "dclass/dc/File.h"
#include "util/EventSender.h"
#include "util/TaskQueue.h"
#include "deps/uvw/uvw.hpp"
#include <unordered_map>
#include <thread>
#include <cassert>
#include <climits>

// An Uberdog represents a global DistributedObject that manages itself, instead of being managed by
//     a StateServer or DatabaseServer.  Uberdogs are typically used for RPC calls, and typically
//     have a single NetworkParticipant endpoint in the cluster that listens to the do_id channel;
//     similar to how an AI server would be connected.
struct Uberdog {
    doid_t do_id;
    const dclass::Class *dcc;
    bool anonymous;
};

extern const dclass::File *g_dcf;
extern std::unique_ptr<Logger> g_logger;
extern std::unique_ptr<ConfigFile> g_config;
extern EventSender g_eventsender;
extern std::unordered_map<doid_t, Uberdog> g_uberdogs;
extern std::thread::id g_main_thread_id;
extern std::shared_ptr<uvw::Loop> g_loop;
