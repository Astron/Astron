#pragma once
#include "messagedirector/MessageDirector.h"
#include "dcparser/dcFile.h"
#include "util/EventSender.h"
#include "Logger.h"
#include "config.h"
#include <boost/asio.hpp>
#include <unordered_map>

// An Uberdog represents a global DistributedObject that manages itself, instead of being managed by
//     a StateServer or DatabaseServer.  Uberdogs are typically used for RPC calls, and typically
//     have a single NetworkParticipant endpoint in the cluster that listens to the do_id channel;
//     similar to how an AI server would be connected.
struct Uberdog
{
	doid_t do_id;
	DCClass *dcc;
	bool anonymous;
};

extern Logger *g_logger;
extern ConfigFile *g_config;
extern DCFile *g_dcf;
extern EventSender g_eventsender;
extern boost::asio::io_service io_service;
extern std::unordered_map<doid_t, Uberdog> g_uberdogs;
