#pragma once
#include <list>
#include <set>
#include <map>
#include <string>
#include "core/logger.h"
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "util/NetworkClient.h"
#include <boost/asio.hpp>
#include <boost/icl/interval_map.hpp>


// Standard type defines
typedef unsigned long long channel_t;

// A ChannelList represents a single channel, or range of channels
//     that a MDParticipant can be subscribed to.
struct ChannelList
{
	channel_t a;
	channel_t b;
	bool is_range;
	bool qualifies(channel_t channel);
	bool operator==(const ChannelList &rhs);
};

class MDParticipantInterface;

// A MessageDirector is the internal networking object for an OpenOTP server-node.
// The MessageDirector recieves message from other servers and routes them to the
//     Client Agent, State Server, DB Server, DB-SS, and other server-nodes as necessary.
class MessageDirector : public NetworkClient
{
public:
	// InitializeMD causes the MessageDirector to start listening for
	//     messages if it hasn't already been initialized.
	void init_network();
	static MessageDirector singleton;

	// handle_datagram accepts any OpenOTP message (a Datagram), and
	//     properly routes it to any subscribed listeners.
	// Message on the CONTROL_MESSAGE channel are processed internally by the MessageDirector.
	void handle_datagram(MDParticipantInterface *p, Datagram &dg);

	// subscribe_channel handles a CONTROL_ADD_CHANNEL control message.
	// (Args) "c": the channel to be added.
	void subscribe_channel(MDParticipantInterface* p, channel_t c);

	// unsubscribe_channel handles a CONTROL_REMOVE_CHANNEL control message.
	// (Args) "c": the channel to be removed.
	void unsubscribe_channel(MDParticipantInterface* p, channel_t c);

	// subscribe_range handles a CONTROL_ADD_RANGE control message.
	// (Args) "lo": the lowest channel to be removed.
	//        "hi": the highest channel to be removed.
	// The range is inclusive.
	void subscribe_range(MDParticipantInterface* p, channel_t lo, channel_t hi);

	// unsubscribe_range handles a CONTROL_REMOVE_RANGE control message.
	// (Args) "lo": the lowest channel to be removed.
	//        "hi": the highest channel to be removed.
	// The range is inclusive.
	void unsubscribe_range(MDParticipantInterface* p, channel_t lo, channel_t hi);

	// logger returns the MessageDirector log category.
	inline LogCategory& logger() { return m_log; }
private:
	MessageDirector();
	LogCategory m_log;

	friend class MDParticipantInterface;
	void add_participant(MDParticipantInterface* participant);
	void remove_participant(MDParticipantInterface* participant);

	// I/O OPERATIONS
	void start_accept(); // Accept new connections from downstream
	void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);
	virtual void network_datagram(Datagram &dg);
	virtual void network_disconnect();

	bool is_client;

	boost::asio::ip::tcp::acceptor *m_acceptor;
	bool m_initialized;

	// Connected participants
	std::list<MDParticipantInterface*> m_participants;

	// Single channel subscriptions
	std::map<channel_t, std::set<MDParticipantInterface*>> m_channel_subscriptions;

	// Range channel subscriptions
	boost::icl::interval_map<channel_t, std::set<MDParticipantInterface*>> m_range_subscriptions;
};



// A MDParticipantInterface is the interface that must be implemented to
//     recieve messages from the MessageDirector.
// MDParticipants might be a StateServer, a single StateServer object, the
//     DB-server, or etc. which are on the node and will be transferred
//     internally.  Another server with its own MessageDirector also would
//     be an MDParticipant.
class MDParticipantInterface
{
public:
	MDParticipantInterface()
	{
		MessageDirector::singleton.add_participant(this);
	}
	virtual ~MDParticipantInterface()
	{
		MessageDirector::singleton.remove_participant(this);
	}

	// handle_datagram should handle a message recieved from the MessageDirector.
	// Implementations of handle_datagram should be non-blocking operations.
	virtual bool handle_datagram(Datagram &dg, DatagramIterator &dgi) = 0;

	// post_remove tells the MDParticipant to handle all of its post remove packets.
	inline void post_remove()
	{
		logger().debug() << "MDParticipant sending post removes..." << std::endl;
		for(auto it = m_post_removes.begin(); it != m_post_removes.end(); ++it)
		{
			Datagram dg(*it);
			send(dg);
		}
	}

	inline std::set<channel_t> &channels() {
		return m_channels;
	}
	inline boost::icl::interval_set<channel_t> &ranges() {
		return m_ranges;
	}

protected:
	inline void send(Datagram &dg)
	{
		MessageDirector::singleton.handle_datagram(this, dg);
	}
	inline void subscribe_channel(channel_t c)
	{
		MessageDirector::singleton.subscribe_channel(this, c);
	}
	inline void unsubscribe_channel(channel_t c)
	{
		MessageDirector::singleton.unsubscribe_channel(this, c);
	}
	inline void subscribe_range(channel_t lo, channel_t hi)
	{
		MessageDirector::singleton.subscribe_range(this, lo, hi);
	}
	inline void unsubscribe_range(channel_t lo, channel_t hi)
	{
		MessageDirector::singleton.unsubscribe_range(this, lo, hi);
	}
	inline void add_post_remove(const std::string &post)
	{
		logger().debug() << "MDParticipant added post remove: " << post << std::endl;
		m_post_removes.push_back(post);
	}
	inline void clear_post_removes()
	{
		logger().spam() << "MDParticipant cleared post removes " << std::endl;
		m_post_removes.clear();
	}
	inline LogCategory logger()
	{
		return MessageDirector::singleton.logger();
	}

private:
	std::set<channel_t> m_channels; // The set of all individually subscribed channels.
	boost::icl::interval_set<channel_t> m_ranges; // The set of all subscribed channel ranges.
	std::vector<std::string> m_post_removes; // The messages to be distributed on unexpected disconnect.
};
