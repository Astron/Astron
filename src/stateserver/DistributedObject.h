#pragma once

class DistributedObject : public MDParticipantInterface
{
public:
	channel_t m_owner;
	unsigned int m_parent_id;
	unsigned int m_zone_id;
	unsigned int m_do_id;
	DCClass *m_dclass;
	std::map<DCField*, std::string> m_ram_fields;
	std::map<DCField*, std::string> m_required_fields;
	channel_t m_ai_channel;
	channel_t m_owner_channel;
	bool m_ai_explicitly_set;
	LogCategory *m_log;

	DistributedObject(unsigned int do_id, DCClass *dclass, unsigned int parent_id, unsigned int zone_id, DatagramIterator &dgi, bool has_other);

	void append_required_data(Datagram &dg);
	void append_other_data(Datagram &dg);

	void send_zone_entry();

	void handle_parent_change(channel_t new_parent);
	void handle_ai_change(channel_t new_channel, bool channel_is_explicit);
	void handle_shard_reset();

	void annihilate();

	void save_field(DCField *field, const std::string &data);
	bool handle_one_update(DatagramIterator &dgi, channel_t sender);
	bool handle_query(Datagram &out, unsigned short field_id);

	virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);
};
