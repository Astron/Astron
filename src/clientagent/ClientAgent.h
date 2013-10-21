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
						uint32_t parent, std::unordered_set<uint32_t> zones) :
				m_interest_id(interest_id), m_client_context(client_context),
				m_parent(parent), m_zones(zones), m_has_total(false), m_total(0)
		{
		}

		bool is_ready(const std::unordered_map<uint32_t, DistributedObject> &dist_objs)
		{
			if(!m_has_total)
			{
				return false;
			}

			uint32_t count = 0;
			for(auto it = dist_objs.begin(); it != dist_objs.end(); ++it)
			{
				const DistributedObject &distobj = it->second;
				if(distobj.parent == m_parent &&
				   (m_zones.find(distobj.zone) != m_zones.end()))
				{
					count++;
				}
			}

			return count >= m_total;
		}

		void store_total(uint32_t total)
		{
			if(!m_has_total)
			{
				m_total = total;
				m_has_total = true;
			}
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
		uint32_t m_next_context;
		std::unordered_set<uint32_t> m_owned_objects;
		std::unordered_set<uint32_t> m_seen_objects;
		std::unordered_map<uint16_t, Interest> m_interests;
		std::unordered_map<uint32_t, InterestOperation*> m_pending_interests;
		std::unordered_map<uint32_t, DistributedObject> m_dist_objs;
	public:
		Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig,
			ChannelTracker *ct);
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
		bool handle_client_object_update_field(DatagramIterator &dgi);
		bool handle_client_object_location(DatagramIterator &dgi);
		bool handle_client_add_interest(DatagramIterator &dgi, bool multiple);
		bool handle_client_remove_interest(DatagramIterator &dgi);
		virtual void network_datagram(Datagram &dg);
		virtual void network_disconnect();
};
