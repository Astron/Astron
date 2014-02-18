#pragma once
#include "util/NetworkClient.h"
#include "messagedirector/MessageDirector.h"

#include <queue>
#include <unordered_set>
#include <unordered_map>

class ClientAgent; // Forward declaration

// The ClientState represents how far in the
// connection process a client has reached.
// A NEW client has just connected and not sent any messages.
// An ANONYMOUS client has completed a handshake.
// An AUTHENTICATED client has been granted access by an Uberdog or AI.
enum ClientState
{
    CLIENT_STATE_NEW,
    CLIENT_STATE_ANONYMOUS,
    CLIENT_STATE_ESTABLISHED
};

// A VisibleObject is used to cache metadata associated with
// a particular object when it becomes visible to a Client.
struct VisibleObject
{
	doid_t id;
	doid_t parent;
	zone_t zone;
	const dclass::Class *dcc;
};

// An Interest represents a Client's interest opened with a
// per-client-unique id, a parent, and one or more interested zones.
struct Interest
{
	uint16_t id;
	doid_t parent;
	std::unordered_set<zone_t> zones;
};

// An InterestOperation represents the process of receiving the entirety of the interest
// within a client.  The InterestOperation stays around until all new visible obterjects from a
// newly created or updated interest have been received and forwarded to the Client.
class InterestOperation
{
	public:
		uint16_t m_interest_id;
		uint32_t m_client_context;
		doid_t m_parent;
		std::unordered_set<zone_t> m_zones;

		bool m_has_total;
		doid_t m_total; // as doid_t because <max_objs_in_zones> == <max_total_objs>

		InterestOperation(uint16_t interest_id, uint32_t client_context,
		                  doid_t parent, std::unordered_set<zone_t> zones);

		bool is_ready(const std::unordered_map<doid_t, VisibleObject> &visible_objects);
		void store_total(doid_t total);
};

class Client : public MDParticipantInterface
{
	public:
		~Client();

		// handle_datagram is the handler for datagrams received from the server
		void handle_datagram(Datagram &dg, DatagramIterator &dgi);
	protected:
		ClientAgent* m_client_agent; // The client_agent handling this client
		ClientState m_state; // current state of the Client state machine
		channel_t m_channel; // current channel client is listening on
		channel_t m_allocated_channel; // channel assigned to client at creation time
		uint32_t m_next_context;

		// m_owned_objects is a list of all objects visible through ownership
		std::unordered_set<doid_t> m_owned_objects;
		// m_seen_objects is a list of all objects visible through interests
		std::unordered_set<doid_t> m_seen_objects;
		// m_historical_objects is a list of all objects which where previously visible, but have
		// had their visibility removed; a historical object may have become visible again
		std::unordered_set<doid_t> m_historical_objects;
		// m_visible_objects is a map which relates all visible objects to VisibleObject metadata.
		std::unordered_map<doid_t, VisibleObject> m_visible_objects;

		// m_interests is a map of interest ids to interests.
		std::unordered_map<uint16_t, Interest> m_interests;
		// m_pending_interests is a map of contexts to in-progress interests.
		std::unordered_map<uint32_t, InterestOperation*> m_pending_interests;
		LogCategory *m_log;

		Client(ClientAgent* client_agent);

		// log_event sends an event to the EventLogger
		void log_event(const std::list<std::string> &event);

		// lookup_object returns the class of the object with a do_id.
		// If that object is not visible to the client, NULL will be returned instead.
		const dclass::Class* lookup_object(doid_t do_id);

		// lookup_interests returns a list of all the interests that a parent-zone pair is visible to.
		std::list<Interest> lookup_interests(doid_t parent_id, zone_t zone_id);

		// add_interest will start a new interest operation and retrieve all the objects an interest
		// from the server, subscribing to each zone in the interest.  If the interest already
		// exists, the interest will be updated with the new zones passed in by the argument.
		void add_interest(Interest &i, uint32_t context);

		// remove_interest find each zone an interest which is not part of another interest and
		// passes it to close_zones() to be removed from the client's visibility.
		void remove_interest(Interest &i, uint32_t context);

		// cloze_zones removes objects visible through the zones from the client and unsubscribes
		// from the associated location channels for those objects.
		void close_zones(doid_t parent, const std::unordered_set<zone_t> &killed_zones);

		// is_historical_object returns true if the object was once visible to the client, but has
		// since been deleted.  The return is still true even if the object has become visible again.
		bool is_historical_object(doid_t do_id);

		/* Client Interface */
		// send_disconnect must close any connections with a connected client; the given reason and
		// error should be forwarded to the client. Additionally, it is recommend to log the event.
		// Handler for CLIENTAGENT_EJECT.
		virtual void send_disconnect(uint16_t reason, const std::string &error_string,
		                             bool security = false);

		// forward_datagram should foward the datagram to the client, or where appopriate parse
		// the packet and send the appropriate equivalent data.
		// Handler for CLIENTAGENT_SEND_DATAGRAM.
		virtual void forward_datagram(Datagram &dg) = 0;

		// handle_drop should immediately disconnect the client without sending any more data.
		// Handler for CLIENTAGENT_DROP.
		virtual void handle_drop() = 0;

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
};
