#pragma once

#include "StateServer.h"

class DistributedObject : public MDParticipantInterface
{
		friend class StateServer;

	public:
		DistributedObject(StateServer *stateserver, doid_t do_id, doid_t parent_id,
		                  zone_t zone_id, DCClass *dclass, DatagramIterator &dgi, bool has_other);
		DistributedObject(StateServer *stateserver, channel_t sender, doid_t do_id,
		                  doid_t parent_id, zone_t zone_id, DCClass *dclass,
		                  std::unordered_map<DCField*, std::vector<uint8_t> > req_fields,
		                  std::map<DCField*, std::vector<uint8_t> > ram_fields);
		~DistributedObject();

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

		inline doid_t get_id()
		{
			return m_do_id;
		}
		inline doid_t get_parent()
		{
			return m_parent_id;
		}
		inline doid_t get_zone()
		{
			return m_zone_id;
		}
		inline channel_t get_location()
		{
			return LOCATION2CHANNEL(m_parent_id, m_zone_id);
		}
		inline channel_t get_ai()
		{
			return m_ai_channel;
		}
		inline channel_t get_owner()
		{
			return m_owner_channel;
		}

	private:
		StateServer *m_stateserver;
		doid_t m_do_id;
		doid_t m_parent_id;
		zone_t m_zone_id;
		DCClass *m_dclass;
		std::unordered_map<DCField*, std::vector<uint8_t> > m_required_fields;
		std::map<DCField*, std::vector<uint8_t> > m_ram_fields;
		channel_t m_ai_channel;
		channel_t m_owner_channel;
		bool m_ai_explicitly_set;
		uint32_t m_next_context;
		doid_t m_child_count;
		std::unordered_map<zone_t, doid_t> m_zone_count;
		LogCategory *m_log;

		void append_required_data(Datagram &dg, bool client_only = false, bool also_owner = false);
		void append_other_data(Datagram &dg, bool client_only = false, bool also_owner = false);

		void send_location_entry(channel_t location);
		void send_ai_entry(channel_t location);
		void send_owner_entry(channel_t location);

		void handle_location_change(doid_t new_parent, zone_t new_zone, channel_t sender);
		void handle_ai_change(channel_t new_ai, channel_t sender, bool channel_is_explicit);

		void annihilate(channel_t sender, bool notify_parent = true);
		void delete_children(channel_t sender);

		void wake_children(); // ask all children for their locations

		void save_field(DCField *field, const std::vector<uint8_t> &data);
		bool handle_one_update(DatagramIterator &dgi, channel_t sender);
		bool handle_one_get(Datagram &out, uint16_t field_id,
		                    bool succeed_if_unset = false, bool is_subfield = false);
};
