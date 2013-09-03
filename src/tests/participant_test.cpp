#include "core/global.h"
#include "messagedirector/messagedirector.h"

class ParticipantTest : public MDParticipantInterface
{
	public:
		ParticipantTest() : MDParticipantInterface()
		{
			MessageDirector::singleton.subscribe_channel(this, 100);
			MessageDirector::singleton.subscribe_channel(this, 200);

			Datagram dg2;
			dg2.add_uint8(1);
			dg2.add_uint64(100);
			dg2.add_string("test");

			MessageDirector::singleton.handle_datagram(&dg2, NULL);

			delete this;
		}

		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
		{
			gLogger->debug() << dgi.read_string() << std::endl;
			return true;
		}
};

ParticipantTest* Participant_Test = new ParticipantTest();
