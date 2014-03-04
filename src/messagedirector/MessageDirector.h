#pragma once
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include "core/Logger.h"
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "util/NetworkClient.h"
#include <boost/asio.hpp>
#include <boost/icl/interval_map.hpp>

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

// A MessageDirector is the internal networking object for an Astron server-node.
// The MessageDirector receives message from other servers and routes them to the
//     Client Agent, State Server, DB Server, DB-SS, and other server-nodes as necessary.
class MessageDirector : public NetworkClient
{
	public:
		// init_network causes the MessageDirector to start listening for
		//     messages if it hasn't already been initialized.
		void init_network();
		static MessageDirector singleton;

		// route_datagram accepts any Astron message (a Datagram), and
		//     properly routes it to any subscribed listeners.
		// Message on the CONTROL_MESSAGE channel are processed internally by the MessageDirector.
		void route_datagram(MDParticipantInterface *p, DatagramHandle dg);

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
		inline LogCategory& logger()
		{
			return m_log;
		}
	private:
		MessageDirector();
		boost::asio::ip::tcp::acceptor *m_acceptor;
		bool m_initialized;
		bool is_client;
		LogCategory m_log;

		// Connected participants
		std::list<MDParticipantInterface*> m_participants;

		// Single channel subscriptions
		std::unordered_map<channel_t, std::set<MDParticipantInterface*> > m_channel_subscriptions;

		// Range channel subscriptions
		boost::icl::interval_map<channel_t, std::set<MDParticipantInterface*> > m_range_subscriptions;


		friend class MDParticipantInterface;
		void add_participant(MDParticipantInterface* participant);
		void remove_participant(MDParticipantInterface* participant);

		// I/O OPERATIONS
		void start_accept(); // Accept new connections from downstream
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);
		virtual void receive_datagram(DatagramHandle dg);
		virtual void receive_disconnect();


};



// A MDParticipantInterface is the interface that must be implemented to
//     receive messages from the MessageDirector.
// MDParticipants might be a StateServer, a single StateServer object, the
//     DB-server, or etc. which are on the node and will be transferred
//     internally.  Another server with its own MessageDirector also would
//     be an MDParticipant.
class MDParticipantInterface
{
		friend class MessageDirector;

	public:
		MDParticipantInterface()
		{
			MessageDirector::singleton.add_participant(this);
		}
		virtual ~MDParticipantInterface()
		{
			MessageDirector::singleton.remove_participant(this);
		}

		// handle_datagram should handle a message received from the MessageDirector.
		// Implementations of handle_datagram should be non-blocking operations.
		virtual void handle_datagram(DatagramHandle dg, DatagramIterator &dgi) = 0;

		// post_remove tells the MDParticipant to handle all of its post remove packets.
		inline void post_remove()
		{
			logger().debug() << "MDParticipant '" << m_name << "' sending post removes..." << std::endl;
			for(auto it = m_post_removes.begin(); it != m_post_removes.end(); ++it)
			{
				route_datagram(*it);
			}
		}

		inline std::set<channel_t> &channels()
		{
			return m_channels;
		}
		inline boost::icl::interval_set<channel_t> &ranges()
		{
			return m_ranges;
		}

	protected:
		inline void route_datagram(DatagramHandle dg)
		{
			MessageDirector::singleton.route_datagram(this, dg);
		}
		inline void subscribe_channel(channel_t c)
		{
			logger().trace() << "MDParticipant '" << m_name << "' subscribed channel: " << c << std::endl;
			MessageDirector::singleton.subscribe_channel(this, c);
		}
		inline void unsubscribe_channel(channel_t c)
		{
			logger().trace() << "MDParticipant '" << m_name << "' unsubscribed channel: " << c << std::endl;
			MessageDirector::singleton.unsubscribe_channel(this, c);
		}
		inline void subscribe_range(channel_t lo, channel_t hi)
		{
			logger().trace() << "MDParticipant '" << m_name << "' subscribed range, "
			                << "lo: " << lo << ", hi: " << hi << std::endl;
			MessageDirector::singleton.subscribe_range(this, lo, hi);
		}
		inline void unsubscribe_range(channel_t lo, channel_t hi)
		{
			logger().trace() << "MDParticipant '" << m_name << "' unsubscribed range, "
			                << "lo: " << lo << ", hi: " << hi << std::endl;
			MessageDirector::singleton.unsubscribe_range(this, lo, hi);
		}
		inline void add_post_remove(const DatagramPtr dg)
		{
			logger().trace() << "MDParticipant '" << m_name << "' added post remove." << std::endl;
			m_post_removes.push_back(dg);
		}
		inline void clear_post_removes()
		{
			logger().trace() << "MDParticipant '" << m_name << "' cleared post removes." << std::endl;
			m_post_removes.clear();
		}
		inline void set_con_name(const std::string &name)
		{
			m_name = name;
		}
		inline void set_con_url(const std::string &url)
		{
			m_url = url;
		}
		inline LogCategory logger()
		{
			return MessageDirector::singleton.logger();
		}

	private:
		std::set<channel_t> m_channels; // The set of all individually subscribed channels.
		boost::icl::interval_set<channel_t> m_ranges; // The set of all subscribed channel ranges.
		std::vector<DatagramHandle> m_post_removes; // The messages to be distributed on unexpected disconnect.
		std::string m_name;
		std::string m_url;

};
