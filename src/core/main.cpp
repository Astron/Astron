#include <string>

#include "global.h"

int main(int argc, char *argv[])
{
	std::string cfg_file;

	MessageDirector *md = new MessageDirector;
	Logger *logger = new Logger;
	ConfigFile *config = new ConfigFile;

	// TODO: Perhaps logging should go to a file specified via command-line switch?
	// And perhaps verbosity should be specified via command-line switch as well?

	logger->info("Loading configuration file...");

	if (argc < 2)
		cfg_file = "openotpd.yml";
	else
		cfg_file = argv[1];
	
	if (!config->load(cfg_file))
	{
		logger->fatal("Could not parse configuration file!");
		return 1;
	}

	// TODO: Load DC, bind/connect MD, and instantiate components.

	// TODO: Run libevent main loop.

	return 0;
}
