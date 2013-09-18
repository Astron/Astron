#include "core/global.h"
#include "messagedirector/MessageDirector.h"
#include <boost/random.hpp>
#include <ctime>

LogCategory perfLog("perftest", "perftest");

#define PERF_NCHANNELS 100000
#define PERF_NDESTCHANNELS 1
#define PERF_NPARTICIPANTS 10
#define PERF_TIME 5.0
#define PERF_DATASIZE 70 //must be atleast 1+(PERF_NDESTCHANNELS*8)
char *data = NULL;

boost::random::mt19937_64 gen;

class PerfParticipant : public MDParticipantInterface
{
	public:
		PerfParticipant() : MDParticipantInterface(), nMessages(0)
		{
			boost::random::uniform_int_distribution<unsigned long long> dist(0,0xFFFFFFFFFFFFFFFF);
			for(unsigned int i = 0; i < PERF_NCHANNELS; ++i)
			{
				subscribe_channel(dist(gen));
			}
		}

		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
		{
		}

		void spam()
		{
			Datagram dg(std::string(data, PERF_DATASIZE));
			send(dg);
			nMessages++;
		}

		unsigned int nMessages;
};

class PerfMain
{
	public:
		PerfMain()
		{
			perfLog.info() << "Starting perf test..." << std::endl;
			perfLog.info() << "Creating random data..." << std::endl;
			data = new char[PERF_DATASIZE];
			data[0] = PERF_NDESTCHANNELS;
			for(unsigned int i = 1; i < PERF_DATASIZE; ++i)
			{
				data[i] = rand()%256;
			}
			perfLog.info() << "Creating PerfParticipants" << std::endl;
			PerfParticipant **participants = new PerfParticipant*[PERF_NPARTICIPANTS];
			for(unsigned int i = 0; i < PERF_NPARTICIPANTS; ++i)
			{
				participants[i] = new PerfParticipant;
			}

			perfLog.info() << "Starting  test..." << std::endl;
			clock_t startTime = clock();
			while((clock()-startTime)/CLOCKS_PER_SEC < PERF_TIME)
			{
				for(unsigned int i = 0; i < PERF_NPARTICIPANTS; ++i)
				{
					participants[i]->spam();
				}
			}
			perfLog.info() << "Test over. Averaging messages..." << std::endl;
			double nMessages = 0;
			for(unsigned int i = 0; i < PERF_NPARTICIPANTS; ++i)
			{
				nMessages += double(participants[i]->nMessages)/double(PERF_NPARTICIPANTS);
			}

			perfLog.info() << "An average of " << nMessages << " messages were processed. "
				"this comes out to be " << nMessages/PERF_TIME << " messages/second" << std::endl;
			perfLog.info() << "Cleaning up..." << std::endl;
			for(unsigned int i = 0; i < PERF_NPARTICIPANTS; ++i)
			{
				delete participants[i];
			}
			delete [] participants;
			delete [] data;
		}
};

PerfMain perftest;
