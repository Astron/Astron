#include <string>
#include <fstream>

#include "global.h"

int main(int argc, char *argv[])
{
	std::string cfg_file;

	gMD = new MessageDirector;
	gLogger = new Logger;
	gConfig = new ConfigFile;
	gDCF = new DCFile;

	// TODO: Perhaps logging should go to a file specified via command-line switch?
	// And perhaps verbosity should be specified via command-line switch as well?

	gLogger->info("Loading configuration file...");

	if (argc < 2)
		cfg_file = "openotpd.yml";
	else
		cfg_file = argv[1];

	std::ifstream file(cfg_file);
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

	//gDCF->read("filename.dc");

	// TODO: Load DC, bind/connect MD, and instantiate components.

	// TODO: Run libevent main loop.

	return 0;
}
