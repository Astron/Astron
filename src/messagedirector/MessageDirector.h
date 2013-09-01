#pragma once
#include <list>
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include <boost/asio.hpp>

class MDParticipantInterface;
class MessageDirector
{
	public:
		void InitializeMD();
		static MessageDirector singleton;
	private:
		MessageDirector();
		std::list<MDParticipantInterface*> m_participants;
		
		friend class MDParticipantInterface;
		
		void add_participant(MDParticipantInterface* participant)
		{
			m_participants.insert(m_participants.end(), participant);
		}
		void remove_participant(MDParticipantInterface* participant)
		{
			m_participants.remove(participant);
		}

		void start_accept();
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);

		boost::asio::ip::tcp::acceptor *m_acceptor;
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
		virtual bool handle_datagram(Datagram *dg, DatagramIterator *dgi) = 0;
};
