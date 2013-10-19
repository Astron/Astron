#pragma once

#include "DBStateServer.h"
#include "DistributedObject.h"

class LoadingObject : public MDParticipantInterface
{
	static uint32_t cls_next_context;

	friend class DBStateServer;

	public:
		LoadingObject(DBStateServer *stateserver, uint32_t do_id, uint32_t parent_id, uint32_t zone_id);
		LoadingObject(DBStateServer *stateserver, uint32_t do_id, uint32_t parent_id, uint32_t zone_id,
		              DCClass *dclass, DatagramIterator &dgi);
		~LoadingObject();

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

	private:
		DBStateServer *m_dbss;
		uint32_t m_do_id;
		uint32_t m_parent_id;
		uint32_t m_zone_id;
		uint32_t m_context;
		LogCategory *m_log;

		// Upstream object data
		DCClass *m_dclass;
		std::unordered_map<DCField*, std::vector<uint8_t>> m_field_updates;
		std::unordered_map<DCField*, std::vector<uint8_t>> m_required_fields;
		std::unordered_map<DCField*, std::vector<uint8_t>> m_ram_fields;

		// Received datagrams while waiting for reply
		std::list<Datagram> m_datagram_queue;

		void inline send_get_object(uint32_t do_id);
		void inline replay_datagrams(DistributedObject* obj);
};