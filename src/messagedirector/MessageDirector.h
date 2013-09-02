#pragma once
#include <list>
#include <map>
#include <string>
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include <boost/asio.hpp>

#define CONTROL_MESSAGE 4001
#define CONTROL_ADD_CHANNEL 2001
#define CONTROL_REMOVE_CHANNEL 2002
#define CONTROL_SET_CON_NAME 2004
#define CONTROL_SET_CON_URL 2005
#define CONTROL_ADD_RANGE 2008
#define CONTROL_REMOVE_RANGE 2009
#define CONTROL_ADD_POST_REMOVE 2010
#define CONTROL_CLEAR_POST_REMOVE 2011

struct ChannelList
{
	unsigned long long a;
	unsigned long long b;
	bool is_range;
	bool qualifies(unsigned long long channel);
	bool operator==(const ChannelList &rhs);
};

class MDParticipantInterface;
class MessageDirector
{
	public:
		void InitializeMD();
		static MessageDirector singleton;

		void handle_datagram(Datagram *dg, MDParticipantInterface *participant);

		void subscribe_channel(MDParticipantInterface* p, unsigned long long a);
		void unsubscribe_channel(MDParticipantInterface* p, unsigned long long a);
		void subscribe_range(MDParticipantInterface* p, unsigned long long a, unsigned long long b);
		void unsubscribe_range(MDParticipantInterface* p, unsigned long long a, unsigned long long b);
	private:
		MessageDirector();
		std::list<MDParticipantInterface*> m_participants;
		std::map<MDParticipantInterface*, std::list<ChannelList> > m_participant_channels;
		std::map<MDParticipantInterface*, std::string> m_post_removes;
		
		friend class MDParticipantInterface;
		
		void add_participant(MDParticipantInterface* participant)
		{
			m_participants.insert(m_participants.end(), participant);
		}
		void remove_participant(MDParticipantInterface* participant)
		{
			if(m_post_removes.find(participant) != m_post_removes.end())
			{
				Datagram dg(m_post_removes[participant]);
				handle_datagram(&dg, participant);
				m_post_removes.erase(m_post_removes.find(participant));
			}
			for(auto it = m_participant_channels[participant].begin(); it != m_participant_channels[participant].end(); ++it)
			{
				Datagram dg;
				dg.add_uint8(1);
				dg.add_uint64(CONTROL_MESSAGE);
				if(it->is_range)
				{
					dg.add_uint16(CONTROL_REMOVE_RANGE);
					dg.add_uint64(it->a);
					dg.add_uint64(it->b);
				}
				else
				{
					dg.add_uint16(CONTROL_REMOVE_CHANNEL);
					dg.add_uint64(it->a);
				}
				handle_datagram(&dg, participant);
			}
			m_participants.remove(participant);
		}

		void start_accept();
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);

		void start_receive();
		void read_handler(const boost::system::error_code &ec, size_t bytes_transferred);

		char *m_buffer;
		unsigned short m_bufsize;
		unsigned short m_bytes_to_go;
		bool m_is_data;

		boost::asio::ip::tcp::acceptor *m_acceptor;
		boost::asio::ip::tcp::socket *m_remote_md;
		bool m_initialized;
};

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
		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi) = 0;
};
