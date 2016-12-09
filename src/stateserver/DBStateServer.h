#pragma once
#include <unordered_set>
#include "StateServer.h"
#include "core/objtypes.h"

/* Helper Functions */
// unpack_db_fields reads the field_count and following fields into a required map and ram map
// from a DBSERVER_GET_ALL_RESP or DBSERVER_GET_FIELDS_RESP message.
// Returns false if unpacking failed for some reason.
bool unpack_db_fields(DatagramIterator &dg, const dclass::Class* dclass,
                      std::unordered_map<const dclass::Field*, std::vector<uint8_t> > &required,
                      FieldValues &ram);

class LoadingObject;

class DBStateServer final : public StateServer
{
    friend class LoadingObject;

  public:
    DBStateServer(RoleConfig roleconfig);

    void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

  private:
    channel_t m_db_channel; // database control channel
    std::unordered_map<doid_t, LoadingObject*> m_loading; // loading but not active objects

    // m_next_context is the next context to send to the db. Invariant: always post-increment.
    uint32_t m_next_context;
    // m_context_datagrams is a map of "context sent to db" to datagram response stubs to send
    // back to the caller. It stores the data used to correctly route the response while the
    // dbss is waiting on the db.
    std::unordered_map<uint32_t, DatagramPtr> m_context_datagrams;

    std::unordered_map<doid_t, std::unordered_set<uint32_t> > m_inactive_loads;

    // handle_activate accepts an activate message and spawns a LoadingObject to handle it.
    void handle_activate(DatagramIterator &dgi, bool has_other);
    void handle_delete_disk(channel_t sender, DatagramIterator &dgi);
    void handle_set_field(DatagramIterator &dgi);
    void handle_set_fields(DatagramIterator &dgi);
    void handle_get_field(channel_t sender, DatagramIterator &dgi);
    void handle_get_field_resp(DatagramIterator &dgi);
    void handle_get_fields(channel_t sender, DatagramIterator &dgi);
    void handle_get_fields_resp(DatagramIterator &dgi);
    void handle_get_all(channel_t sender, DatagramIterator &dgi);
    void handle_get_all_resp(DatagramIterator &dgi);
    void handle_get_activated(channel_t sender, DatagramIterator &dgi);

    // receive_object gives responsibility of a DistributedObject to the dbss
    // primarily used by a LoadingObject when the object is finished loading.
    void receive_object(DistributedObject* obj);
    // discard_loader tells the dbss to forget about a LoadingObject, either
    // because the object finished loading, or because the object failed to load.
    void discard_loader(doid_t do_id);
    // is_expected_context returns true if we're expecting a dbss response with the context
    inline bool is_expected_context(uint32_t context);
    // is_activated_object returns true if the doid is an active or loading object.
    inline bool is_activated_object(doid_t);
};
