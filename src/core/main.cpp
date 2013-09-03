#include <string>
#include <cstring>
#include <fstream>
#include <vector>

#include "global.h"

ConfigVariable<std::vector<std::string>> dc_files("general/dc_files", std::vector<std::string>());

int main(int argc, char *argv[])
{
	std::string cfg_file;

	//TODO: Perhaps verbosity should be specified via command-line switch?
	if (argc < 2)
	{
		cfg_file = "openotpd.yml";
	}
	else
	{
		cfg_file = "openotpd.yml";
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i],  "-config") == 0 && i + 1 < argc) 
			{
				cfg_file = argv[++i];
			}
			else if (strcmp(argv[i], "-log") == 0 && i + 1 < argc)
			{
				delete gLogger;
				gLogger = new Logger(argv[++i]);
			}
		}
	}

	gLogger->info() << "Loading configuration file..." << std::endl;


	std::ifstream file(cfg_file.c_str());
	if(!file.is_open())
	{
		gLogger->fatal() << "Failed to open configuration file." << std::endl;
		return 1;
	}
	
	if (!gConfig->load(file))
	{
		gLogger->fatal() << "Could not parse configuration file!" << std::endl;
		return 1;
	}
	file.close();

	std::vector<std::string> dc_file_names = dc_files.get_val();
	for(auto it = dc_file_names.begin(); it != dc_file_names.end(); ++it)
	{
		if(!gDCF->read(*it))
		{
			gLogger->fatal() << "Could not read DC file " << *it << std::endl;
			return 1;
		}
	}

	MessageDirector::singleton.InitializeMD();

	try
	{
		io_service.run();
	}
	catch(std::exception &e)
	{
		gLogger->fatal() << "Exception from the network io service: "
		                 << e.what() << std::endl;
	}

	//gDCF->read("filename.dc");

	// TODO: Load DC, bind/connect MD, and instantiate components.

	// TODO: Run libevent main loop.

	return 0;
}
