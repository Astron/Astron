#include "core/global.h"
#include "messagedirector/messagedirector.h"

class ParticipantTest : public MDParticipantInterface
{
	public:
		ParticipantTest() : MDParticipantInterface()
		{
			Datagram dg;
			dg.add_uint8(1);
			dg.add_uint64(CONTROL_MESSAGE);
			dg.add_uint16(CONTROL_SET_CHANNEL);
			dg.add_uint64(100);

			MessageDirector::singleton.handle_datagram(&dg, this);

			Datagram dg4;
			dg4.add_uint8(1);
			dg4.add_uint64(CONTROL_MESSAGE);
			dg4.add_uint16(CONTROL_SET_CHANNEL);
			dg4.add_uint64(200);

			MessageDirector::singleton.handle_datagram(&dg4, this);

			Datagram dg2;
			dg2.add_uint8(2);
			dg2.add_uint64(100);
			dg2.add_uint64(200);
			dg2.add_string("test");

			MessageDirector::singleton.handle_datagram(&dg2, NULL);

			Datagram dg3;
			dg3.add_uint8(1);
			dg3.add_uint64(CONTROL_MESSAGE);
			dg3.add_uint16(CONTROL_REMOVE_CHANNEL);
			dg3.add_uint64(100);

			MessageDirector::singleton.handle_datagram(&dg3, this);

			Datagram dg5;
			dg5.add_uint8(1);
			dg5.add_uint64(CONTROL_MESSAGE);
			dg5.add_uint16(CONTROL_REMOVE_CHANNEL);
			dg5.add_uint64(200);

			MessageDirector::singleton.handle_datagram(&dg5, this);
		}

		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
		{
			gLogger->debug() << dgi.read_string() << std::endl;
			return true;
		}
};

ParticipantTest participant_test;
