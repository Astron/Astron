#pragma once
#include <list>
#include "../core/datagram.h"
#include "../core/datagramIterator.h"

class MDParticipantInterface;
class MessageDirector
{
	public:
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
};

class MDParticipantInterface
{
	public:
		MDParticipantInterface()
		{
			MessageDirector::singleton.add_participant(this);
		}
		~MDParticipantInterface()
		{
			MessageDirector::singleton.remove_participant(this);
		}
		virtual bool handle_datagram(Datagram *dg, DatagramIterator *dgi) = 0;
};