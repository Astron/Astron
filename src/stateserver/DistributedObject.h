#pragma once
#include "StateServer.h"
#include "core/objtypes.h"

class DistributedObject : public MDParticipantInterface
{
    friend class StateServer;

  public:
    DistributedObject(StateServer *stateserver, doid_t do_id, doid_t parent_id, zone_t zone_id,
                      const dclass::Class *dclass, DatagramIterator &dgi, bool has_other);
    DistributedObject(StateServer *stateserver, channel_t sender, doid_t do_id,
                      doid_t parent_id, zone_t zone_id, const dclass::Class *dclass,
                      UnorderedFieldValues& req_fields, FieldValues& ram_fields);
    ~DistributedObject();

    virtual void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

    inline doid_t get_id() const
    {
        return m_do_id;
    }
    inline doid_t get_parent() const
    {
        return m_parent_id;
    }
    inline doid_t get_zone() const
    {
        return m_zone_id;
    }
    inline channel_t get_location() const
    {
        return location_as_channel(m_parent_id, m_zone_id);
    }
    inline channel_t get_ai() const
    {
        return m_ai_channel;
    }
    inline channel_t get_owner() const
    {
        return m_owner_channel;
    }

  private:
    StateServer *m_stateserver;
    doid_t m_do_id;
    doid_t m_parent_id;
    zone_t m_zone_id;
    const dclass::Class *m_dclass;
    UnorderedFieldValues m_required_fields;
    FieldValues m_ram_fields;
    channel_t m_ai_channel;
    channel_t m_owner_channel;
    bool m_ai_explicitly_set;
    bool m_parent_synchronized;
    uint32_t m_next_context;
    std::unordered_map<zone_t, std::unordered_set<doid_t>> m_zone_objects;
    LogCategory *m_log;

    void append_required_data(DatagramPtr dg, bool client_only = false, bool also_owner = false);
    void append_other_data(DatagramPtr dg, bool client_only = false, bool also_owner = false);

    void send_interest_entry(channel_t location, uint32_t context);
    void send_location_entry(channel_t location);
    void send_ai_entry(channel_t location);
    void send_owner_entry(channel_t location);

    void handle_location_change(doid_t new_parent, zone_t new_zone, channel_t sender);
    void handle_ai_change(channel_t new_ai, channel_t sender, bool channel_is_explicit);

    void annihilate(channel_t sender, bool notify_parent = true);
    void delete_children(channel_t sender);

    void wake_children(); // ask all children for their locations

    void save_field(const dclass::Field *field, const std::vector<uint8_t> &data);
    bool handle_one_update(DatagramIterator &dgi, channel_t sender);
    bool handle_one_get(DatagramPtr out, uint16_t field_id,
                        bool succeed_if_unset = false, bool is_subfield = false);
};
