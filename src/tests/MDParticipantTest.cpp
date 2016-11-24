#include "core/global.h"
#include "messagedirector/MessageDirector.h"

class MDParticipantTest : public MDParticipantInterface
{
  public:
    MDParticipantTest() : MDParticipantInterface()
    {
        subscribe_channel(100);
        subscribe_channel(200);

        Datagram dg2;
        dg2.add_uint8(2);
        dg2.add_channel(100);
        dg2.add_channel(200);
        dg2.add_string("test");

        MessageDirector::singleton.handle_datagram(nullptr, dg2);

        unsubscribe_channel(100);
        unsubscribe_channel(200);

        /*MessageDirector::singleton.subscribe_range(this, 1000, 2000);
        MessageDirector::singleton.unsubscribe_range(this, 1500, 1700);*/
    }

    virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
    {
        g_logger->log(LogSeverity::LSEVERITY_DEBUG) << dgi.read_string() << std::endl;
    }
};

MDParticipantTest unittest_mdparticipant;
