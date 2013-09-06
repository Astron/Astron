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
			dg2.add_uint8(2);
			dg2.add_uint64(100);
			dg2.add_uint64(200);
			dg2.add_string("test");

			MessageDirector::singleton.handle_datagram(NULL, dg2);

			MessageDirector::singleton.unsubscribe_channel(this, 100);
			MessageDirector::singleton.unsubscribe_channel(this, 200);

			/*MessageDirector::singleton.subscribe_range(this, 1000, 2000);
			MessageDirector::singleton.unsubscribe_range(this, 1500, 1700);*/
		}

		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
		{
			gLogger->log(LogSeverity::LSEVERITY_DEBUG) << dgi.read_string() << std::endl;
		}
};

ParticipantTest participant_test;
