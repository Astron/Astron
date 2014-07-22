#pragma once
#include <list>
#include <set>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ChannelMap.h"
#include "core/Logger.h"
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "net/NetworkAcceptor.h"
#include <boost/asio.hpp>
#include <boost/icl/interval_map.hpp>

class MDParticipantInterface;
class MDUpstream;

// A MessageDirector is the internal networking object for an Astron server-node.
// The MessageDirector receives message from other servers and routes them to the
//     Client Agent, State Server, DB Server, DB-SS, and other server-nodes as necessary.
class MessageDirector : public ChannelMap
{
	public:
		~MessageDirector();

		// init_network causes the MessageDirector to start listening for
		//     messages if it hasn't already been initialized.
		void init_network();
		static MessageDirector singleton;

		// route_datagram accepts any Astron message (a Datagram), and
		//     properly routes it to any subscribed listeners.
		// Message on the CONTROL_MESSAGE channel are processed internally by the MessageDirector.
		void route_datagram(MDParticipantInterface *p, DatagramHandle dg);

		// logger returns the MessageDirector log category.
		inline LogCategory& logger()
		{
			return m_log;
		}

		// For MDUpstream (and subclasses) to call.
		void receive_datagram(DatagramHandle dg);
		void receive_disconnect();

	protected:
		virtual void on_add_channel(channel_t c);

		virtual void on_remove_channel(channel_t c);

		virtual void on_add_range(channel_t lo, channel_t hi);

		virtual void on_remove_range(channel_t lo, channel_t hi);

	private:
		MessageDirector();

		bool m_initialized;

		NetworkAcceptor *m_net_acceptor;
		MDUpstream *m_upstream;

		// Connected participants
		std::list<MDParticipantInterface*> m_participants;

		// Threading stuff:
		bool m_shutdown;
		std::thread *m_thread;
		std::mutex m_participants_lock;
		std::mutex m_messages_lock;
		std::queue<std::pair<MDParticipantInterface *, DatagramHandle>> m_messages;
		std::condition_variable m_cv;
        std::thread::id m_main_thread;
		void process_datagram(MDParticipantInterface *p, DatagramHandle dg);
		void routing_thread();
		void shutdown_threading();

		LogCategory m_log;

		friend class MDParticipantInterface;
		void add_participant(MDParticipantInterface* participant);
		void remove_participant(MDParticipantInterface* participant);

		// I/O OPERATIONS
		void handle_connection(boost::asio::ip::tcp::socket *socket);
};



// A MDParticipantInterface is the interface that must be implemented to
//     receive messages from the MessageDirector.
// MDParticipants might be a StateServer, a single StateServer object, the
//     DB-server, or etc. which are on the node and will be transferred
//     internally.  Another server with its own MessageDirector also would
//     be an MDParticipant.
class MDParticipantInterface : public ChannelSubscriber
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
		inline void unsubscribe_all()
		{
			logger().trace() << "MDParticipant '" << m_name << "' unsubscribing from all.\n";
			MessageDirector::singleton.unsubscribe_all(this);
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
		std::vector<DatagramHandle> m_post_removes; // The messages to be distributed on unexpected disconnect.
		std::string m_name;
		std::string m_url;

};

// This class abstractly represents an "upstream" link on the Message Director.
// All messages routed on the Message Director will be sent to the upstream link,
// except for messages that originated on the link to begin with.
class MDUpstream
{
	public:
		virtual void subscribe_channel(channel_t c) = 0;
		virtual void unsubscribe_channel(channel_t c) = 0;
		virtual void subscribe_range(channel_t lo, channel_t hi) = 0;
		virtual void unsubscribe_range(channel_t lo, channel_t hi) = 0;
		virtual void handle_datagram(DatagramHandle dg) = 0;
};
