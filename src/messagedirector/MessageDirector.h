#pragma once
#include <list>
#include <set>
#include <map>
#include <string>
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include <boost/asio.hpp>
#include <boost/icl/interval_map.hpp>

// Message-channel constants
#define CONTROL_MESSAGE 4001

// Message-type constants
#define CONTROL_ADD_CHANNEL 2001
#define CONTROL_REMOVE_CHANNEL 2002
#define CONTROL_SET_CON_NAME 2004
#define CONTROL_SET_CON_URL 2005
#define CONTROL_ADD_RANGE 2008
#define CONTROL_REMOVE_RANGE 2009
#define CONTROL_ADD_POST_REMOVE 2010
#define CONTROL_CLEAR_POST_REMOVE 2011

// A ChannelList represents a single channel, or range of channels
//     that a MDParticipant can be subscribed to.
struct ChannelList
{
	unsigned long long a;
	unsigned long long b;
	bool is_range;
	bool qualifies(unsigned long long channel);
	bool operator==(const ChannelList &rhs);
};

class MDParticipantInterface;

// A MessageDirector is the internal networking object for an OpenOTP server-node.
// The MessageDirector recieves message from other servers and routes them to the
//     Client Agent, State Server, DB Server, DB-SS, and other server-nodes as necessary.
class MessageDirector
{
	public:
		// InitializeMD causes the MessageDirector to start listening for
		//     messages if it hasn't already been initialized.
		void InitializeMD();
		static MessageDirector singleton;

		// handle_datagram accepts any OpenOTP message (a Datagram), and
		//     properly routes it to any subscribed listeners.
		// Message on the CONTROL_MESSAGE channel are processed internally by the MessageDirector.
		void handle_datagram(Datagram *dg, MDParticipantInterface *participant);

		// subscribe_channel handles a CONTROL_ADD_CHANNEL control message.
		// (Args) "a": the channel to be added.
		void subscribe_channel(MDParticipantInterface* p, unsigned long long a);

		// unsubscribe_channel handles a CONTROL_REMOVE_CHANNEL control message.
		// (Args) "a": the channel to be removed.
		void unsubscribe_channel(MDParticipantInterface* p, unsigned long long a);

		// subscribe_range handles a CONTROL_ADD_RANGE control message.
		// (Args) "a": the lowest channel to be removed.
		//        "b": the highest channel to be removed.
		// The range is inclusive.
		void subscribe_range(MDParticipantInterface* p, unsigned long long a, unsigned long long b);

		// unsubscribe_range handles a CONTROL_REMOVE_RANGE control message.
		// (Args) "a": the lowest channel to be removed.
		//        "b": the highest channel to be removed.
		// The range is inclusive.
		void unsubscribe_range(MDParticipantInterface* p, unsigned long long a, unsigned long long b);
	private:
		MessageDirector();

		// All participants associated with the MD
		std::list<MDParticipantInterface*> m_participants;

		// All single channel susbcriptions
		std::map<unsigned long long, std::set<MDParticipantInterface*>> m_channel_subscriptions;

		// All range channel subscriptions
		boost::icl::interval_map<unsigned long long, std::set<MDParticipantInterface*>> m_range_subscriptions;
		
		friend class MDParticipantInterface;
		
		void add_participant(MDParticipantInterface* participant);
		void remove_participant(MDParticipantInterface* participant);

		void start_accept(); // Accept new connections from downstream
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);

		void start_receive(); // Recieve message from upstream
		void read_handler(const boost::system::error_code &ec, size_t bytes_transferred);

		// should_control_upstream determines whether CONTROL_MESSAGES should be routed upstream
		bool should_control_upstream(ChannelList);

		char *m_buffer;
		unsigned short m_bufsize;
		unsigned short m_bytes_to_go;
		bool m_is_data;

		boost::asio::ip::tcp::acceptor *m_acceptor;
		boost::asio::ip::tcp::socket *m_remote_md;
		bool m_initialized;
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
		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi) = 0;

		inline std::list<ChannelList> channels()
		{
			return m_channels;
		}
		inline std::string post_remove()
		{
			return m_post_remove;
		}

		inline void set_post_remove(std::string s)
		{
			m_post_remove = s;
		}

	private:
		std::list<ChannelList> m_channels; // A list of all connected channels.
		std::string m_post_remove; // The message to be distributed on unexpected disconnect.
};