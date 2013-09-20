#pragma once

#include "StateServer.h"

class DistributedObject : public MDParticipantInterface
{
		friend class StateServer;

	public:
		DistributedObject(StateServer *stateserver, uint32_t do_id, DCClass *dclass, uint32_t parent_id, uint32_t zone_id, DatagramIterator &dgi, bool has_other);
		~DistributedObject();

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

	private:
		StateServer *m_stateserver;
		uint32_t m_do_id;
		DCClass *m_dclass;
		uint32_t m_parent_id;
		uint32_t m_zone_id;
		std::map<DCField*, std::vector<uint8_t>> m_ram_fields; // TODO: Fix for std::unordered_map
		std::unordered_map<DCField*, std::vector<uint8_t>> m_required_fields;
		channel_t m_ai_channel;
		channel_t m_owner_channel;
		bool m_ai_explicitly_set;
		LogCategory *m_log;

		void append_required_data(Datagram &dg);
		void append_other_data(Datagram &dg);

		void send_zone_entry(channel_t destination);

		void handle_parent_change(channel_t new_parent);
		void handle_ai_change(channel_t new_channel, bool channel_is_explicit);
		void handle_shard_reset();

		void annihilate();

		void save_field(DCField *field, const std::vector<uint8_t> &data);
		bool handle_one_update(DatagramIterator &dgi, channel_t sender);
		bool handle_query(Datagram &out, uint16_t field_id);
};
