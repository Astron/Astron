#include "Client.h"
#include "ClientMessages.h"
#include "ClientFactory.h"
#include "ClientAgent.h"
#include "net/NetworkClient.h"
#include "core/global.h"
#include "core/msgtypes.h"
#include "config/constraints.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"
#include "util/Timeout.h"

using namespace std;
using dclass::Class;
using dclass::Field;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;


static ConfigGroup astronclient_config("libastron", ca_client_config);
static ConfigVariable<bool> relocate_owned("relocate", false, astronclient_config);
static ConfigVariable<string> interest_permissions("add_interest", "visible", astronclient_config);
static BooleanValueConstraint relocate_is_boolean(relocate_owned);

static ConfigVariable<bool> send_hash_to_client("send_hash", true, astronclient_config);
static ConfigVariable<bool> send_version_to_client("send_version", true, astronclient_config);

static ConfigVariable<uint64_t> write_buffer_size("write_buffer_size", 256 * 1024,
        astronclient_config);
static ConfigVariable<unsigned int> write_timeout_ms("write_timeout_ms", 5000, astronclient_config);

//by default, have heartbeat disabled.
static ConfigVariable<long> heartbeat_timeout_config("heartbeat_timeout", 0, astronclient_config);

static bool is_permission_level(const string& str)
{
    return (str == "visible" || str == "disabled" || str == "enabled");
}
static ConfigConstraint<string> valid_permission_level(is_permission_level, interest_permissions,
        "Permissions for add_interest must be one of 'visible', 'enabled', 'disabled'.");

enum InterestPermission {
    INTERESTS_ENABLED,
    INTERESTS_VISIBLE,
    INTERESTS_DISABLED
};

class AstronClient : public Client, public NetworkHandler
{
  private:
    std::shared_ptr<NetworkClient> m_client;
    ConfigNode m_config;
    bool m_clean_disconnect;
    bool m_relocate_owned;
    bool m_send_hash;
    bool m_send_version;
    InterestPermission m_interests_allowed;

    //Heartbeat
    long m_heartbeat_timeout;
    std::shared_ptr<Timeout> m_heartbeat_timer = nullptr;

  public:
    AstronClient(ConfigNode config, ClientAgent* client_agent, tcp::socket *socket,
                 const tcp::endpoint &remote, const tcp::endpoint &local) :
        Client(config, client_agent), m_client(std::make_shared<NetworkClient>(this)),
        m_config(config),
        m_clean_disconnect(false), m_relocate_owned(relocate_owned.get_rval(config)),
        m_send_hash(send_hash_to_client.get_rval(config)),
        m_send_version(send_version_to_client.get_rval(config)),
        m_heartbeat_timeout(heartbeat_timeout_config.get_rval(config))
    {
        m_client->initialize(socket, remote, local);

        initialize();
    }

    AstronClient(ConfigNode config, ClientAgent* client_agent,
                 ssl::stream<tcp::socket> *stream,
                 const tcp::endpoint &remote, const tcp::endpoint &local) :
        Client(config, client_agent), m_client(std::make_shared<NetworkClient>(this)),
        m_config(config),
        m_clean_disconnect(false), m_relocate_owned(relocate_owned.get_rval(config)),
        m_send_hash(send_hash_to_client.get_rval(config)),
        m_send_version(send_version_to_client.get_rval(config)),
        m_heartbeat_timeout(heartbeat_timeout_config.get_rval(config))
    {
        m_client->initialize(stream, remote, local);

        initialize();
    }

    void heartbeat_timeout()
    {
        lock_guard<recursive_mutex> lock(m_client_lock);
        send_disconnect(CLIENT_DISCONNECT_NO_HEARTBEAT,
                        "Server timed out while waiting for heartbeat.");
    }

    void initialize()
    {
        //If heartbeat, start the heartbeat timer now.
        if(m_heartbeat_timeout != 0) {
            m_heartbeat_timer = std::make_shared<Timeout>(m_heartbeat_timeout,
                                std::bind(&AstronClient::heartbeat_timeout,
                                          this));
            m_heartbeat_timer->start();
        }

        // Set interest permissions
        string permission_level = interest_permissions.get_rval(m_config);
        if(permission_level == "enabled") {
            m_interests_allowed = INTERESTS_ENABLED;
        } else if(permission_level == "visible") {
            m_interests_allowed = INTERESTS_VISIBLE;
        } else {
            m_interests_allowed = INTERESTS_DISABLED;
        }

        // Set NetworkClient config
        m_client->set_write_timeout(write_timeout_ms.get_rval(m_config));
        m_client->set_write_buffer(write_buffer_size.get_rval(m_config));

        stringstream ss;
        ss << "Client (" << m_client->get_remote().address().to_string()
           << ":" << m_client->get_remote().port() << ", " << m_channel << ")";
        m_log->set_name(ss.str());
        set_con_name(ss.str());

        // Create event for EventLogger
        LoggedEvent event("client-connected");

        // Add remote endpoint to log
        ss.str(""); // empty the stream
        ss << m_client->get_remote().address().to_string()
           << ":" << m_client->get_remote().port();
        event.add("remote_address", ss.str());

        // Add local endpoint to log
        ss.str(""); // empty the stream
        ss << m_client->get_local().address().to_string()
           << ":" << m_client->get_local().port();
        event.add("local_address", ss.str());

        // Log created event
        log_event(event);
    }

    // send_disconnect must close any connections with a connected client; the given reason and
    // error should be forwarded to the client. Additionaly, it is recommend to log the event.
    // Handler for CLIENTAGENT_EJECT.
    virtual void send_disconnect(uint16_t reason, const string &error_string, bool security = false)
    {
        if(m_client->is_connected()) {
            Client::send_disconnect(reason, error_string, security);

            DatagramPtr resp = Datagram::create();
            resp->add_uint16(CLIENT_EJECT);
            resp->add_uint16(reason);
            resp->add_string(error_string);
            m_client->send_datagram(resp);

            m_clean_disconnect = true;
            m_client->disconnect();
        }
    }

    // receive_datagram is the handler for datagrams received over the network from a Client.
    virtual void receive_datagram(DatagramHandle dg)
    {
        lock_guard<recursive_mutex> lock(m_client_lock);
        DatagramIterator dgi(dg);
        try {
            switch(m_state) {
            // Client has just connected and should only send "CLIENT_HELLO".
            case CLIENT_STATE_NEW:
                handle_pre_hello(dgi);
                break;
            // Client has sent "CLIENT_HELLO" and can now access anonymous uberdogs.
            case CLIENT_STATE_ANONYMOUS:
                handle_pre_auth(dgi);
                break;
            // An Uberdog or AI has declared the Client authenticated and the client
            // can now interact with the server cluster normally.
            case CLIENT_STATE_ESTABLISHED:
                handle_authenticated(dgi);
                break;
            }
        } catch(const DatagramIteratorEOF&) {
            // Occurs when a handler attempts to read past end of datagram
            send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM,
                            "Datagram unexpectedly ended while iterating.");
            return;
        } catch(const DatagramOverflow&) {
            // Occurs when a handler attempts to prepare or forward a datagram to be sent
            // internally and, the resulting datagram is larger than the max datagram size.
            send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM,
                            "ClientDatagram too large to be routed on MD.", true);
            return;
        }

        if(dgi.get_remaining()) {
            // All client handlers should consume all data in the datagram (for validation and security).
            // If the handler read all the data it expected, and some remains, the datagram was sent with
            // additional junk data on the end.
            send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Datagram contains excess data.", true);
            return;
        }
    }

    // receive_disconnect is called when the Client closes the tcp
    //     connection or otherwise when the tcp connection is lost.
    // Note: In the Astron client protocol, the server is normally
    //       responsible for terminating the connection.
    virtual void receive_disconnect(const boost::system::error_code &ec)
    {
        lock_guard<recursive_mutex> lock(m_client_lock);

        if(!m_clean_disconnect) {
            LoggedEvent event("client-lost");
            event.add("reason", ec.message());
            log_event(event);
        }

        if(m_heartbeat_timer != nullptr) {
            m_heartbeat_timer->cancel();
        }

        annihilate();
    }

    // forward_datagram should foward the datagram to the client, or where appopriate parse
    // the packet and send the appropriate equivalent data.
    // Handler for CLIENTAGENT_SEND_DATAGRAM.
    virtual void forward_datagram(DatagramHandle dg)
    {
        m_client->send_datagram(dg);
    }

    // handle_drop should immediately disconnect the client without sending any more data.
    // Handler for CLIENTAGENT_DROP.
    virtual void handle_drop()
    {
        m_clean_disconnect = true;
        m_client->disconnect();
    }

    // handle_add_interest should inform the client of an interest added by the server.
    virtual void handle_add_interest(const Interest& i, uint32_t context)
    {
        bool multiple = i.zones.size() > 1;

        DatagramPtr resp = Datagram::create();
        resp->add_uint16(multiple ? CLIENT_ADD_INTEREST_MULTIPLE : CLIENT_ADD_INTEREST);
        resp->add_uint32(context);
        resp->add_uint16(i.id);
        resp->add_doid(i.parent);
        if(multiple) {
            resp->add_uint16(i.zones.size());
        }
        for(auto it = i.zones.begin(); it != i.zones.end(); ++it) {
            resp->add_zone(*it);
        }
        m_client->send_datagram(resp);
    }

    // handle_remove_interest should inform the client an interest was removed by the server.
    virtual void handle_remove_interest(uint16_t interest_id, uint32_t context)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_REMOVE_INTEREST);
        resp->add_uint32(context);
        resp->add_uint16(interest_id);
        m_client->send_datagram(resp);
    }

    // handle_add_object should inform the client of a new object. The datagram iterator
    // provided starts at the 'required fields' data, and may have optional fields following.
    // Handler for OBJECT_ENTER_LOCATION (an object, enters the Client's interest).
    virtual void handle_add_object(doid_t do_id, doid_t parent_id, zone_t zone_id, uint16_t dc_id,
                                   DatagramIterator &dgi, bool other)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER : CLIENT_ENTER_OBJECT_REQUIRED);
        resp->add_doid(do_id);
        resp->add_location(parent_id, zone_id);
        resp->add_uint16(dc_id);
        resp->add_data(dgi.read_remainder());
        m_client->send_datagram(resp);
    }

    // handle_add_ownership should inform the client it has control of a new object. The datagram
    // iterator provided starts at the 'required fields' data, and may have 'optional fields'.
    // Handler for OBJECT_ENTER_OWNER (an object, enters the Client's ownership).
    virtual void handle_add_ownership(doid_t do_id, doid_t parent_id, zone_t zone_id, uint16_t dc_id,
                                      DatagramIterator &dgi, bool other)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER
                         : CLIENT_ENTER_OBJECT_REQUIRED_OWNER);
        resp->add_doid(do_id);
        resp->add_location(parent_id, zone_id);
        resp->add_uint16(dc_id);
        resp->add_data(dgi.read_remainder());
        m_client->send_datagram(resp);
    }

    // handle_set_field should inform the client that the field has been updated.
    virtual void handle_set_field(doid_t do_id, uint16_t field_id, DatagramIterator &dgi)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_OBJECT_SET_FIELD);
        resp->add_doid(do_id);
        resp->add_uint16(field_id);
        resp->add_data(dgi.read_remainder());
        m_client->send_datagram(resp);
    }

    // handle_set_fields should inform the client that a group of fields has been updated.
    virtual void handle_set_fields(doid_t do_id, uint16_t num_fields, DatagramIterator &dgi)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_OBJECT_SET_FIELDS);
        resp->add_doid(do_id);
        resp->add_uint16(num_fields);
        resp->add_data(dgi.read_remainder());
        m_client->send_datagram(resp);
    }

    // handle_change_location should inform the client that the objects location has changed.
    virtual void handle_change_location(doid_t do_id, doid_t new_parent, zone_t new_zone)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_OBJECT_LOCATION);
        resp->add_doid(do_id);
        resp->add_location(new_parent, new_zone);
        m_client->send_datagram(resp);
    }

    // handle_remove_object should send a mesage to remove the object from the connected client.
    // Handler for cases where an object is no longer visible to the client;
    //     for example, when it changes zone, leaves visibility, or is deleted.
    virtual void handle_remove_object(doid_t do_id)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_OBJECT_LEAVING);
        resp->add_doid(do_id);
        m_client->send_datagram(resp);
    }

    // handle_remove_ownership should notify the client it no has control of the object.
    // Handle when the client loses ownership of an object.
    virtual void handle_remove_ownership(doid_t do_id)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_OBJECT_LEAVING_OWNER);
        resp->add_doid(do_id);
        m_client->send_datagram(resp);
    }

    // handle_interest_done is called when all of the objects from an opened interest have been
    // received. Typically, informs the client that a particular group of objects is loaded.
    virtual void handle_interest_done(uint16_t interest_id, uint32_t context)
    {
        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_DONE_INTEREST_RESP);
        resp->add_uint32(context);
        resp->add_uint16(interest_id);
        m_client->send_datagram(resp);
    }

    // Client has just connected and should only send "CLIENT_HELLO"
    // Only handles one message type, so it does not need to be split up.
    virtual void handle_pre_hello(DatagramIterator &dgi)
    {
        uint16_t msg_type = dgi.read_uint16();
        if(msg_type != CLIENT_HELLO) {
            send_disconnect(CLIENT_DISCONNECT_NO_HELLO, "First packet is not CLIENT_HELLO");
            return;
        }

        //Now that message type is confirmed, reset timeout watchdog.
        handle_client_heartbeat();

        uint32_t dc_hash = dgi.read_uint32();
        string version = dgi.read_string();

        if(version != m_client_agent->get_version()) {
            stringstream ss;
            ss << "Client version mismatch: client=" << version;
            if(m_send_version) {
                ss << ", server=" << m_client_agent->get_version();
            }

            send_disconnect(CLIENT_DISCONNECT_BAD_VERSION, ss.str());

            return;
        }

        const static uint32_t expected_hash = m_client_agent->get_hash();
        if(dc_hash != expected_hash) {
            stringstream ss;
            ss << "Client DC hash mismatch: client=0x" << hex << dc_hash;
            if(m_send_hash) {
                ss << ", server=0x" << expected_hash;
            }
            send_disconnect(CLIENT_DISCONNECT_BAD_DCHASH, ss.str());
            return;
        }

        DatagramPtr resp = Datagram::create();
        resp->add_uint16(CLIENT_HELLO_RESP);
        m_client->send_datagram(resp);

        m_state = CLIENT_STATE_ANONYMOUS;
    }

    // Client has sent "CLIENT_HELLO" and can now access anonymous uberdogs.
    virtual void handle_pre_auth(DatagramIterator &dgi)
    {
        uint16_t msg_type = dgi.read_uint16();
        switch(msg_type) {
        case CLIENT_DISCONNECT: {
            LoggedEvent event("client-disconnected");
            log_event(event);

            m_clean_disconnect = true;
            m_client->disconnect();
        }
        break;
        case CLIENT_OBJECT_SET_FIELD:
            handle_client_object_update_field(dgi);
            break;
        case CLIENT_HEARTBEAT:
            handle_client_heartbeat();
            break;
        default:
            stringstream ss;
            ss << "Message type " << msg_type << " not allowed prior to authentication.";
            send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
            return;
        }
    }

    // An Uberdog or AI has declared the Client authenticated and the client
    // can now interact with the server cluster normally.
    virtual void handle_authenticated(DatagramIterator &dgi)
    {
        uint16_t msg_type = dgi.read_uint16();
        switch(msg_type) {
        case CLIENT_DISCONNECT: {
            LoggedEvent event("client-disconnected");
            log_event(event);

            m_clean_disconnect = true;
            m_client->disconnect();
        }
        break;
        case CLIENT_OBJECT_SET_FIELD:
            handle_client_object_update_field(dgi);
            break;
        case CLIENT_OBJECT_LOCATION:
            handle_client_object_location(dgi);
            break;
        case CLIENT_ADD_INTEREST:
            handle_client_add_interest(dgi, false);
            break;
        case CLIENT_ADD_INTEREST_MULTIPLE:
            handle_client_add_interest(dgi, true);
            break;
        case CLIENT_REMOVE_INTEREST:
            handle_client_remove_interest(dgi);
            break;
        case CLIENT_HEARTBEAT:
            handle_client_heartbeat();
            break;
        default:
            stringstream ss;
            ss << "Message type " << msg_type << " not valid.";
            send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
            return;
        }
    }

    // handle_client_heartbeat should ensure this client does not get reset for the current interval.
    // Handler for CLIENT_HEARTBEAT message
    virtual void handle_client_heartbeat()
    {
        if(m_heartbeat_timer != nullptr) {
            m_heartbeat_timer->reset();
        }
    }

    // handle_client_object_update_field occurs when a client sends an OBJECT_SET_FIELD
    virtual void handle_client_object_update_field(DatagramIterator &dgi)
    {
        doid_t do_id = dgi.read_doid();
        uint16_t field_id = dgi.read_uint16();

        // Get class of object from cache
        const Class *dcc = lookup_object(do_id);

        // If the class couldn't be found, error out:
        if(!dcc) {
            if(is_historical_object(do_id)) {
                // The client isn't disconnected in this case because it could be a delayed
                // message, we also have to skip to the end so a disconnect overside_datagram
                // is not sent.
                // TODO: Allow configuration to limit how long historical objects remain,
                //       for example with a timeout or bad-message limit.
                dgi.skip(dgi.get_remaining());
            } else {
                stringstream ss;
                ss << "Client tried to send update to nonexistent object " << do_id;
                send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
            }
            return;
        }

        // If the client is not in the ESTABLISHED state, it may only send updates
        // to anonymous UberDOGs.
        if(m_state != CLIENT_STATE_ESTABLISHED) {
            if(g_uberdogs.find(do_id) == g_uberdogs.end() || !g_uberdogs[do_id].anonymous) {
                stringstream ss;
                ss << "Client tried to send update to non-anonymous object "
                   << dcc->get_name() << "(" << do_id << ")";
                send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION, ss.str(), true);
                return;
            }
        }

        // Check that the client sent a field that actually exists in the class.
        const Field *field = dcc->get_field_by_id(field_id);
        if(!field) {
            stringstream ss;
            ss << "Client tried to send update for nonexistent field " << field_id << " to object "
               << dcc->get_name() << "(" << do_id << ")";
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
            return;
        }

        // Check that the client is actually allowed to send updates to this field
        bool is_owned = m_owned_objects.find(do_id) != m_owned_objects.end();
        if(!field->has_keyword("clsend") && !(is_owned && field->has_keyword("ownsend"))) {
            auto send_it = m_fields_sendable.find(do_id);
            if(send_it == m_fields_sendable.end() ||
               send_it->second.find(field_id) == send_it->second.end()) {
                stringstream ss;
                ss << "Client tried to send update for non-sendable field: "
                   << dcc->get_name() << "(" << do_id << ")." << field->get_name();
                send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
                return;
            }
        }

        // If an exception occurs while unpacking data it will be handled by
        // receive_datagram and the client will be dc'd with "truncated datagram".
        vector<uint8_t> data;
        dgi.unpack_field(field, data);

        // If an exception occurs while packing data it will be handled by
        // receive_datagram and the client will be dc'd with "oversized datagram".
        DatagramPtr resp = Datagram::create();
        resp->add_server_header(do_id, m_channel, STATESERVER_OBJECT_SET_FIELD);
        resp->add_doid(do_id);
        resp->add_uint16(field_id);
        resp->add_data(data);
        route_datagram(resp);
    }

    // handle_client_object_location occurs when a client sends an OBJECT_LOCATION message.
    // When sent by the client, this represents a request to change the object's location.
    virtual void handle_client_object_location(DatagramIterator &dgi)
    {
        // Check the client is configured to allow client-relocates
        if(!m_relocate_owned) {
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE,
                            "Owned object relocation is disabled by server.", true);
            return;
        }
        // Check that the object the client is trying manipulate actually exists
        doid_t do_id = dgi.read_doid();
        if(m_visible_objects.find(do_id) == m_visible_objects.end()) {
            if(is_historical_object(do_id)) {
                // The client isn't disconnected in this case because it could be a delayed
                // message, we also have to skip to the end so a disconnect overside_datagram
                // is not sent.
                // TODO: Allow configuration to limit how long historical objects remain,
                //       for example with a timeout or bad-message limit.
                dgi.skip(dgi.get_remaining());
            } else {
                stringstream ss;
                ss << "Client tried to manipulate unknown object " << do_id;
                send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
            }
            return;
        }

        // Check that the client is actually allowed to change the object's location
        bool is_owned = m_owned_objects.find(do_id) != m_owned_objects.end();
        if(!is_owned) {
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE,
                            "Can't relocate an object the client doesn't own", true);
            return;
        }

        // Update the object's location
        DatagramPtr dg = Datagram::create(do_id, m_channel, STATESERVER_OBJECT_SET_LOCATION);
        dg->add_doid(dgi.read_doid()); // Parent
        dg->add_zone(dgi.read_zone()); // Zone
        route_datagram(dg);
    }

    // handle_client_add_interest occurs is called when the client adds an interest.
    virtual void handle_client_add_interest(DatagramIterator &dgi, bool multiple)
    {
        if(m_interests_allowed == INTERESTS_DISABLED) {
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_INTEREST,
                            "Client is not allowed to add interests.", true);
            return;
        }

        uint32_t context = dgi.read_uint32();

        Interest i;
        build_interest(dgi, multiple, i);
        if(m_interests_allowed == INTERESTS_VISIBLE && !lookup_object(i.parent)) {
            stringstream ss;
            ss << "Cannot add interest to parent with id " << i.parent
               << " because parent is not visible to client.";
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_INTEREST, ss.str(), true);
            return;
        }
        add_interest(i, context);
    }

    // handle_client_remove_interest is called when the client removes an interest.
    virtual void handle_client_remove_interest(DatagramIterator &dgi)
    {
        if(m_interests_allowed == INTERESTS_DISABLED) {
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_INTEREST,
                            "Client is not allowed to remove interests.", true);
            return;
        }

        uint32_t context = dgi.read_uint32();
        uint16_t id = dgi.read_uint16();

        // check the interest actually exists to be removed
        if(m_interests.find(id) == m_interests.end()) {
            send_disconnect(CLIENT_DISCONNECT_GENERIC, "Tried to remove a non-existing intrest", true);
            return;
        }

        Interest &i = m_interests[id];
        if(m_interests_allowed == INTERESTS_VISIBLE && !lookup_object(i.parent)) {
            stringstream ss;
            ss << "Cannot remove interest for parent with id " << i.parent
               << " because parent is not visible to client.";
            send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_INTEREST, ss.str(), true);
            return;
        }
        remove_interest(i, context);
    }

    virtual const std::string get_remote_address()
    {
        return m_client->get_remote().address().to_string();
    }

    virtual uint16_t get_remote_port()
    {
        return m_client->get_remote().port();
    }

    virtual const std::string get_local_address()
    {
        return m_client->get_local().address().to_string();
    }

    virtual uint16_t get_local_port()
    {
        return m_client->get_local().port();
    }
};

static ClientType<AstronClient> astron_client_fact("libastron");
