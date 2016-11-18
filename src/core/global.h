#pragma once
#include "Logger.h"
#include "config/ConfigVariable.h"
#include "dclass/dc/File.h"
#include "util/EventSender.h"
#include <boost/asio.hpp>
#include <unordered_map>

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
extern boost::asio::io_service io_service;
extern EventSender g_eventsender;
extern std::unordered_map<doid_t, Uberdog> g_uberdogs;
