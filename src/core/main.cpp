#include <string>
#include <cstring>
#include <fstream>

#include "global.h"

int main(int argc, char *argv[])
{
	std::string cfg_file;

	gLogger = new Logger;
	gConfig = new ConfigFile;
	gDCF = new DCFile;


	//TODO: Perhaps verbosity should be specified via command-line switch?
	if (argc < 2)
	{
		cfg_file = "openotpd.yml";
	}
	else
	{
		cfg_file = "openotpd.yml";
		std::string log_file = "none";
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i],  "-config") == 0 && i + 1 < argc) 
			{
				cfg_file = argv[++i];
			}
			else if (strcmp(argv[i], "-log") == 0 && i + 1 < argc)
			{
				log_file = argv[++i];
			}
		}
		gLogger = new Logger(log_file);
	}

	gLogger->info("Loading configuration file...");


	std::ifstream file(cfg_file.c_str());
	if(!file.is_open())
	{
		gLogger->fatal("Failed to open configuration file.");
		return 1;
	}
	
	if (!gConfig->load(file))
	{
		gLogger->fatal("Could not parse configuration file!");
		return 1;
	}
	file.close();

	MessageDirector::singleton.InitializeMD();

	try
	{
		io_service.run();
	}
	catch(std::exception &e)
	{
		gLogger->fatal("Exception from the network io service %s", e.what());
	}

	//gDCF->read("filename.dc");

	// TODO: Load DC, bind/connect MD, and instantiate components.

	// TODO: Run libevent main loop.

	return 0;
}
