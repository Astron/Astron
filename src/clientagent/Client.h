#pragma once
#include "net/NetworkClient.h"
#include "messagedirector/MessageDirector.h"
#include "util/EventSender.h"
#include "util/Timeout.h"

#include <queue>
#include <memory>
#include <unordered_set>
#include <unordered_map>

#include <mutex>

class ClientAgent; // Forward declaration

// The ClientState represents how far in the
// connection process a client has reached.
// A NEW client has just connected and not sent any messages.
// An ANONYMOUS client has completed a handshake.
// An AUTHENTICATED client has been granted access by an Uberdog or AI.
enum ClientState {
    CLIENT_STATE_NEW,
    CLIENT_STATE_ANONYMOUS,
    CLIENT_STATE_ESTABLISHED
};

struct DeclaredObject {
    doid_t id;
    const dclass::Class *dcc;
};

// A VisibleObject is used to cache metadata associated with
// a particular object when it becomes visible to a Client.
struct VisibleObject : DeclaredObject {
    doid_t parent;
    zone_t zone;
};

// An Interest represents a Client's interest opened with a
// per-client-unique id, a parent, and one or more interested zones.
struct Interest {
    uint16_t id;
    doid_t parent;
    std::unordered_set<zone_t> zones;
};


class Client; // forward declaration
// An InterestOperation represents the process of receiving the entirety of the interest
// within a client.  The InterestOperation stays around until all new visible objects from a
// newly created or updated interest have been received and forwarded to the Client.
class InterestOperation
{
  public:
    Client *m_client;

    uint16_t m_interest_id;
    uint32_t m_client_context;
    uint32_t m_request_context;
    doid_t m_parent;
    std::unordered_set<zone_t> m_zones;
    std::set<channel_t> m_callers;

    std::shared_ptr<Timeout> m_timeout;

    bool m_has_total = false;
    doid_t m_total = 0; // as doid_t because <max_objs_in_zones> == <max_total_objs>

    std::list<DatagramHandle> m_pending_generates;
    std::list<DatagramHandle> m_pending_datagrams;

    InterestOperation(Client *client, unsigned long timeout,
                      uint16_t interest_id, uint32_t client_context, uint32_t request_context,
                      doid_t parent, std::unordered_set<zone_t> zones, channel_t caller);
    ~InterestOperation();

    bool is_ready();
    void set_expected(doid_t total);
    void queue_expected(DatagramHandle dg);
    void queue_datagram(DatagramHandle dg);
    void finish(bool is_timeout = false);
    void timeout();

  private:
    bool m_finished = false;
};

class Client : public MDParticipantInterface
{
    friend class InterestOperation;
  public:
    virtual ~Client();

    // handle_datagram is the handler for datagrams received from the server
    void handle_datagram(DatagramHandle dg, DatagramIterator &dgi);

  protected:
    std::recursive_mutex m_client_lock;     // The lock guarding the client.
    ClientAgent* m_client_agent;            // The ClientAgent handling this client
    ClientState m_state = CLIENT_STATE_NEW; // Current state of the Client state machine
    channel_t m_channel = 0;                // Current channel client is listening on
    channel_t m_allocated_channel = 0;      // Channel assigned to client at creation time
    uint32_t m_next_context = 1;

    // m_owned_objects is a list of all objects visible through ownership
    std::unordered_set<doid_t> m_owned_objects;
    // m_seen_objects is a list of all objects visible through interests
    std::unordered_set<doid_t> m_seen_objects;
    // m_session_objects is a list of objects that are automatically cleaned up when the
    // client disconnects.  Additionally, these objects are presumed to be required for the
    // client, so if one is delete, the client is dropped.
    std::unordered_set<doid_t> m_session_objects;
    // m_historical_objects is a list of all objects which where previously visible, but have
    // had their visibility removed; a historical object may have become visible again
    std::unordered_set<doid_t> m_historical_objects;
    // m_visible_objects is a map which relates all visible objects to VisibleObject metadata.
    std::unordered_map<doid_t, VisibleObject> m_visible_objects;
    // m_declared_objects is a map of declared objects to their metadata.
    std::unordered_map<doid_t, DeclaredObject> m_declared_objects;
    // m_pending_objects is a map of doids for objects that we need to buffer dg's for
    std::unordered_map<doid_t, uint32_t> m_pending_objects;

    // m_interests is a map of interest ids to interests.
    std::unordered_map<uint16_t, Interest> m_interests;
    // m_pending_interests is a map of contexts to in-progress interests.
    std::unordered_map<uint32_t, InterestOperation*> m_pending_interests;
    // m_fields_sendable is a map of DoIds to sendable field sets.
    std::unordered_map<uint16_t, std::unordered_set<uint16_t> > m_fields_sendable;
    // If we did not receive a pointer from the Client Agent, we create one ourselves
    // in m_log_owner and have m_log point to that. This way, we are guarenteed that
    // the instance will always be freed, no matter if we are using the logger from
    // the Client Agent or in here.
    LogCategory *m_log;
    // new LogCategory, incase we did not receive one from the Client Agent
    std::unique_ptr<LogCategory> m_log_owner;

    Client(ConfigNode config, ClientAgent* client_agent);

    // annihilate should be called to delete the client after the client has-left/disconnected.
    void annihilate();

    // log_event sends an event to the EventLogger
    void log_event(LoggedEvent &event);

    // lookup_object returns the class of the object with a do_id.
    // If that object is not visible to the client, nullptr will be returned instead.
    const dclass::Class* lookup_object(doid_t do_id);

    // lookup_interests returns a list of all the interests that a parent-zone pair is visible to.
    std::list<Interest> lookup_interests(doid_t parent_id, zone_t zone_id);

    // build_interest will build an interest from a datagram. It is expected that the datagram
    // iterator is positioned such that next item to be read is the interest_id.
    void build_interest(DatagramIterator &dgi, bool multiple, Interest &out);

    // add_interest will start a new interest operation and retrieve all the objects an interest
    // from the server, subscribing to each zone in the interest.  If the interest already
    // exists, the interest will be updated with the new zones passed in by the argument.
    // The caller is used to specify a server channel that should be informed when the interest
    // operation is completed; typically that is not specified by subclasses.
    void add_interest(Interest &i, uint32_t context, channel_t caller = 0);

    // remove_interest find each zone an interest which is not part of another interest and
    // passes it to close_zones() to be removed from the client's visibility.
    void remove_interest(Interest &i, uint32_t context, channel_t caller = 0);

    // cloze_zones removes objects visible through the zones from the client and unsubscribes
    // from the associated location channels for those objects.
    void close_zones(doid_t parent, const std::unordered_set<zone_t> &killed_zones);

    // is_historical_object returns true if the object was once visible to the client, but has
    // since been deleted.  The return is still true even if the object has become visible again.
    bool is_historical_object(doid_t do_id);


    // handle_object_entrance is a common handler for object entrance. the DGI should be positioned
    // at the start of the do_id parameter
    void handle_object_entrance(DatagramIterator &dgi, bool other);

    // try_queue_pending checks the object against m_pending_objects, and if the objects is
    // involved in a pending iop, queues the datagram for later sending, and returns true
    inline bool try_queue_pending(doid_t do_id, DatagramHandle dg);

    /* Client Interface */
    // send_disconnect must close any connections with a connected client; the given reason and
    // error should be forwarded to the client. Additionally, it is recommend to log the event.
    // Handler for CLIENTAGENT_EJECT.
    virtual void send_disconnect(uint16_t reason, const std::string &error_string,
                                 bool security = false);

    // forward_datagram should foward the datagram to the client, or where appopriate parse
    // the packet and send the appropriate equivalent data.
    // Handler for CLIENTAGENT_SEND_DATAGRAM.
    virtual void forward_datagram(DatagramHandle dg) = 0;

    // handle_drop should immediately disconnect the client without sending any more data.
    // Handler for CLIENTAGENT_DROP.
    virtual void handle_drop() = 0;

    // handle_add_interest should inform the client of an interest added by the server.
    virtual void handle_add_interest(const Interest &i, uint32_t context) = 0;

    // handle_remove_interest should inform the client an interest was removed by the server.
    virtual void handle_remove_interest(uint16_t interest_id, uint32_t context) = 0;

    // handle_add_object should inform the client of a new object. The datagram iterator
    // provided starts at the 'required fields' data, and may have optional fields following.
    // Handler for OBJECT_ENTER_LOCATION (an object, enters the Client's interest).
    virtual void handle_add_object(doid_t do_id, doid_t parent_id, zone_t zone_id,
                                   uint16_t dc_id, DatagramIterator &dgi, bool other = false) = 0;

    // handle_add_ownership should inform the client it has control of a new object. The datagram
    // iterator provided starts at the 'required fields' data, and may have 'optional fields'.
    // Handler for OBJECT_ENTER_OWNER (an object, enters the Client's ownership).
    virtual void handle_add_ownership(doid_t do_id, doid_t parent_id, zone_t zone_id,
                                      uint16_t dc_id, DatagramIterator &dgi, bool other = false) = 0;

    // handle_set_field should inform the client that the field has been updated.
    virtual void handle_set_field(doid_t do_id, uint16_t field_id,
                                  DatagramIterator &dgi) = 0;

    // handle_set_fields should inform the client that a group of fields has been updated.
    virtual void handle_set_fields(doid_t do_id, uint16_t num_fields, DatagramIterator &dgi) = 0;

    // handle_change_location should inform the client that the objects location has changed.
    virtual void handle_change_location(doid_t do_id, doid_t new_parent, zone_t new_zone) = 0;

    // handle_remove_object should send a mesage to remove the object from the connected client.
    // Handler for cases where an object is no longer visible to the client;
    //     for example, when it changes zone, leaves visibility, or is deleted.
    virtual void handle_remove_object(doid_t do_id) = 0;

    // handle_remove_ownership should notify the client it no has control of the object.
    // Handle when the client loses ownership of an object.
    virtual void handle_remove_ownership(doid_t do_id) = 0;

    // handle_interest_done is called when all of the objects from an opened interest have been
    // received. Typically, informs the client that a particular group of objects is loaded.
    virtual void handle_interest_done(uint16_t interest_id, uint32_t context) = 0;

    // get_remote_address and friends are set by AstronClient, because we have no access
    // to m_remote from Client
    virtual const std::string get_remote_address() = 0;
    virtual uint16_t get_remote_port() = 0;
    virtual const std::string get_local_address() = 0;
    virtual uint16_t get_local_port() = 0;

  private:
    // notify_interest_done send a CLIENTAGENT_DONE_INTEREST_RESP to the
    // interest operation's caller, if one has been set.
    void notify_interest_done(uint16_t interest_id, channel_t caller);
    void notify_interest_done(const InterestOperation* iop);
};
