#pragma once
#include "DBStateServer.h"
#include "DistributedObject.h"

class LoadingObject : public MDParticipantInterface
{
		friend class DBStateServer;

	public:
		LoadingObject(DBStateServer *stateserver, doid_t do_id, doid_t parent_id, zone_t zone_id,
		              const std::unordered_set<uint32_t> &contexts = std::unordered_set<uint32_t>());
		LoadingObject(DBStateServer *stateserver, doid_t do_id, doid_t parent_id, zone_t zone_id,
		              dclass::Class *dclass, DatagramIterator &dgi,
		              const std::unordered_set<uint32_t> &contexts = std::unordered_set<uint32_t>());
		~LoadingObject();

		void begin();

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

	private:
		DBStateServer *m_dbss;
		doid_t m_do_id;
		doid_t m_parent_id;
		zone_t m_zone_id;
		uint32_t m_context;
		LogCategory *m_log;

		// Upstream object data
		dclass::Class *m_dclass;
		std::unordered_map<dclass::Field*, std::vector<uint8_t> > m_field_updates;
		std::unordered_map<dclass::Field*, std::vector<uint8_t> > m_required_fields;
		std::map<dclass::Field*, std::vector<uint8_t> > m_ram_fields;
		std::unordered_set<uint32_t> m_valid_contexts;

		// Received datagrams while waiting for reply
		std::list<Datagram> m_datagram_queue;
		bool m_is_loaded;

		void inline send_get_object(doid_t do_id);
		void inline replay_datagrams(DistributedObject* obj);
};
