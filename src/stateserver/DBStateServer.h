#pragma once
#include <unordered_map>

#include "StateServer.h"

class LoadingObject;

class DBStateServer : public StateServer
{
	friend class LoadingObject;

	public:
		DBStateServer(RoleConfig roleconfig);
		~DBStateServer();


		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

	private:
		channel_t m_db_channel;
		std::unordered_map<uint32_t, LoadingObject*> m_loading;

		void handle_activate(DatagramIterator &dgi, bool has_other);
		void receive_object(DistributedObject* obj);
		void discard_loader(uint32_t do_id);
};