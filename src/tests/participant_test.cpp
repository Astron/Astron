#include "messagedirector/messagedirector.h"

class ParticipantTest : public MDParticipantInterface
{
	public:
		virtual bool handle_datagram(Datagram *dg, DatagramIterator *dgi)
		{
			return true;
		}
};

ParticipantTest participant_test;
