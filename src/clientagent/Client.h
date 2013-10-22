#pragma once
#include "util/NetworkClient.h"
#include "messagedirector/MessageDirector.h"

#include <queue>
#include <unordered_set>
#include <unordered_map>

enum ClientState
{
	CLIENT_STATE_NEW,
	CLIENT_STATE_ANONYMOUS,
	CLIENT_STATE_ESTABLISHED
};

struct VisibleObject
{
	uint32_t id;
	uint32_t parent;
	uint32_t zone;
	DCClass *dcc;
};

struct Interest
{
	uint16_t id;
	uint32_t parent;
	std::unordered_set<uint32_t> zones;
};

class InterestOperation
{
	public:
		uint16_t m_interest_id;
		uint32_t m_client_context;
		uint32_t m_parent;
		std::unordered_set<uint32_t> m_zones;

		bool m_has_total;
		uint32_t m_total;

		InterestOperation(uint16_t interest_id, uint32_t client_context,
		                  uint32_t parent, std::unordered_set<uint32_t> zones);

		bool is_ready(const std::unordered_map<uint32_t, VisibleObject> &dist_objs);
		void store_total(uint32_t total);
};

class ChannelTracker
{
	public:
		ChannelTracker(channel_t min, channel_t max);

		channel_t alloc_channel();
		void free_channel(channel_t channel);

	private:
		channel_t m_next;
		channel_t m_max;
		std::queue<channel_t> m_unused_channels;
};

class Client : public NetworkClient, public MDParticipantInterface
{
	private:
		ClientState m_state;
		LogCategory *m_log;
		std::string m_server_version;
		ChannelTracker *m_ct;
		channel_t m_channel;
		channel_t m_allocated_channel;
		bool m_is_channel_allocated;
		bool m_clean_disconnect;
		uint32_t m_next_context;
		std::unordered_set<uint32_t> m_owned_objects;
		std::unordered_set<uint32_t> m_seen_objects;
		std::unordered_map<uint16_t, Interest> m_interests;
		std::unordered_map<uint32_t, InterestOperation*> m_pending_interests;
		std::unordered_map<uint32_t, VisibleObject> m_dist_objs;
	public:
		Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, 
		       const std::string &server_version, ChannelTracker *ct);
		virtual ~Client();
		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi);
	protected:
		void send_event(const std::list<std::string> &event);
		void send_disconnect(uint16_t reason, const std::string &error_string, bool security=false);
		virtual void handle_pre_hello(DatagramIterator &dgi);
		virtual void handle_pre_auth(DatagramIterator &dgi);
		virtual void handle_authenticated(DatagramIterator &dgi);
		DCClass *lookup_object(uint32_t do_id);
		std::list<Interest> lookup_interests(uint32_t parent_id, uint32_t zone_id);
		void close_zones(uint32_t parent, const std::unordered_set<uint32_t> &killed_zones);
		void add_interest(Interest &i, uint32_t context);
		void remove_interest(Interest &i, uint32_t context);
	private:
		void handle_client_object_update_field(DatagramIterator &dgi);
		void handle_client_object_location(DatagramIterator &dgi);
		void handle_client_add_interest(DatagramIterator &dgi, bool multiple);
		void handle_client_remove_interest(DatagramIterator &dgi);
		virtual void network_datagram(Datagram &dg);
		virtual void network_disconnect();
};
