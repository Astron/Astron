#pragma once
#include "util/Datagram.h"
#include "util/DatagramIterator.h"

#include <unordered_set>

enum ClientState
{
	CLIENT_STATE_NEW,
	CLIENT_STATE_ANONYMOUS,
	CLIENT_STATE_ESTABLISHED
};

class ChannelTracker;

struct DistributedObject
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
	uint32_t context;
	bool has_total;
	uint32_t total;
	bool has_announced;

	Interest() : id(0), parent(0), zones(0), context(0), has_total(false),
	             total(0), has_announced(false)
	{
	}

	bool is_ready(const std::unordered_map<uint32_t, DistributedObject> &dist_objs)
	{
		if(!has_total)
		{
			return false;
		}

		uint32_t count = 0;
		for(auto it = dist_objs.begin(); it != dist_objs.end(); ++it)
		{
			const DistributedObject &distobj = it->second;
			if(distobj.parent == parent &&
			   (zones.find(distobj.zone) != zones.end()))
			{
				count++;
			}
		}

		return count >= total;
	}
};

class Client : public NetworkClient, public MDParticipantInterface
{
	private:
		ClientState m_state;
		LogCategory *m_log;
		RoleConfig m_roleconfig;
		ChannelTracker *m_ct;
		channel_t m_channel;
		channel_t m_allocated_channel;
		bool m_is_channel_allocated;
		std::unordered_set<uint32_t> m_owned_objects;
		std::unordered_set<uint32_t> m_seen_objects;
		std::unordered_map<uint16_t, Interest> m_interests;
		std::unordered_map<uint32_t, DistributedObject> m_dist_objs;
	public:
		Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig,
			ChannelTracker *ct);
		virtual ~Client();
		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi);
	protected:
		void send_disconnect(uint16_t reason, const std::string &error_string, bool security=false);
		virtual void handle_pre_hello(DatagramIterator &dgi);
		virtual void handle_pre_auth(DatagramIterator &dgi);
		virtual void handle_authenticated(DatagramIterator &dgi);
		DCClass *lookup_object(uint32_t do_id);
		std::list<Interest> lookup_interests(uint32_t parent_id, uint32_t zone_id);
	private:
		std::list<uint32_t> add_interest(Interest &i);
		void remove_interest(Interest &i, uint32_t id);
		void alter_interest(Interest &i, uint16_t id);
		void request_zone_objects(uint32_t context, uint32_t parent, std::list<uint32_t> new_zones);
		bool handle_client_object_update_field(DatagramIterator &dgi);
		bool handle_client_object_location(DatagramIterator &dgi);
		bool handle_client_add_interest(DatagramIterator &dgi, bool multiple);
		bool handle_client_remove_interest(DatagramIterator &dgi);
		virtual void network_datagram(Datagram &dg);
		virtual void network_disconnect();
};
