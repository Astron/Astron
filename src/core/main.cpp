#include <string>
#include <cstring>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

#include "global.h"
#include "RoleFactory.h"
#include "dclass/file/read.h"
#include "dclass/Class.h"
using dclass::Class;

static ConfigVariable<std::vector<std::string> > dc_files("general/dc_files", std::vector<std::string>());
LogCategory mainlog("main", "Main");
void printHelp(std::ostream &s);


int main(int argc, char *argv[])
{
	std::string cfg_file;

	if(argc < 2)
	{
		cfg_file = "astrond.yml";
	}
	else
	{
		cfg_file = "astrond.yml";
		LogSeverity sev = g_logger->get_min_severity();
		for(int i = 1; i < argc; i++)
		{
			if((strcmp(argv[i], "--log") == 0 || strcmp(argv[i], "-L") == 0) && i + 1 < argc)
			{
				delete g_logger;
				g_logger = new Logger(argv[++i], sev);
			}
			else if((strcmp(argv[i], "--loglevel") == 0 || strcmp(argv[i], "-l") == 0) && i + 1 < argc)
			{
				std::string llstr(argv[++i]);
				if(llstr == "packet")
				{
					sev = LSEVERITY_PACKET;
					g_logger->set_min_severity(sev);
				}
				else if(llstr == "trace")
				{
					sev = LSEVERITY_TRACE;
					g_logger->set_min_severity(sev);
				}
				else if(llstr == "debug")
				{
					sev = LSEVERITY_DEBUG;
					g_logger->set_min_severity(sev);
				}
				else if(llstr == "info")
				{
					sev = LSEVERITY_INFO;
					g_logger->set_min_severity(sev);
				}
				else if(llstr == "warning")
				{
					sev = LSEVERITY_INFO;
					g_logger->set_min_severity(sev);
				}
				else if(llstr == "security")
				{
					sev = LSEVERITY_SECURITY;
					g_logger->set_min_severity(sev);
				}
				else if(llstr != "error" && llstr != "fatal")
				{
					std::cerr << "Unknown log-level \"" << llstr << "\"." << std::endl;
					printHelp(std::cerr);
					exit(1);
				}
			}
			else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
			{
				printHelp(std::cout);
				exit(0);
			}
			else if(argv[i][0] == '-')
			{
				std::cerr << "Unrecognized option \"" << std::string(argv[i]) << "\"." << std::endl;
				printHelp(std::cerr);
				exit(1);
			}
		}
		if(argv[argc - 1][0] != '-')
		{
			cfg_file = argv[argc - 1];
			// seperate path
			boost::filesystem::path p(cfg_file);
			boost::filesystem::path dir = p.parent_path();
			std::string filename = p.filename().string();
			std::string dir_str = dir.string();

			// change directory
			boost::filesystem::current_path(dir_str);
			
			cfg_file = filename; 	
		}
	}

	mainlog.info() << "Loading configuration file..." << std::endl;


	std::ifstream file(cfg_file.c_str());
	if(!file.is_open())
	{
		mainlog.fatal() << "Failed to open configuration file." << std::endl;
		return 1;
	}

	if(!g_config->load(file))
	{
		mainlog.fatal() << "Could not parse configuration file!" << std::endl;
		return 1;
	}
	file.close();

	dclass::File* dcf = new dclass::File();
	dcf->add_keyword("required");
	dcf->add_keyword("ram");
	dcf->add_keyword("db");
	dcf->add_keyword("broadcast");
	dcf->add_keyword("clrecv");
	dcf->add_keyword("clsend");
	dcf->add_keyword("ownsend");
	dcf->add_keyword("ownrecv");
	dcf->add_keyword("airecv");
	std::vector<std::string> dc_file_names = dc_files.get_val();
	for(auto it = dc_file_names.begin(); it != dc_file_names.end(); ++it)
	{
		bool ok = dclass::append(dcf, *it);
		if(!ok)
		{
			mainlog.fatal() << "Could not read DC file " << *it << std::endl;
			return 1;
		}
	}
	g_dcf = dcf;

	// Initialize configured MessageDirector
	MessageDirector::singleton.init_network();
	g_eventsender.init();

	// Load uberdog metadata from configuration
	YAML::Node udnodes = g_config->copy_node()["uberdogs"];
	if(!udnodes.IsNull())
	{
		for(auto it = udnodes.begin(); it != udnodes.end(); ++it)
		{
			YAML::Node udnode = *it;
			Uberdog ud;

			// Get the uberdog's class
			const Class* dcc = g_dcf->get_class_by_name(udnode["class"].as<std::string>());
			if(!dcc)
			{
				// Make sure it exists
				mainlog.fatal() << "For uberdog " << udnode["id"].as<doid_t>()
								<< " Distributed class " << udnode["class"].as<std::string>()
				                << " does not exist!" << std::endl;
				exit(1);
			}

			// Setup uberdog
			ud.dcc = dcc;
			ud.anonymous = udnode["anonymous"].as<bool>();
			g_uberdogs[udnode["id"].as<doid_t>()] = ud;
		}
	}

	// Initialize configured roles
	YAML::Node node = g_config->copy_node();
	node = node["roles"];
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		RoleFactory::singleton.instantiate_role((*it)["type"].as<std::string>(), *it);
	}

	try
	{
		io_service.run();
	}
	catch(std::exception &e)
	{
		mainlog.fatal() << "Exception from the network io service: "
		                << e.what() << std::endl;
	}

	return 0;
}

// printHelp outputs the cli help-text to the given stream.
void printHelp(std::ostream &s)
{
	s << "Usage: astrond [OPTION]... [CONFIG]" << std::endl
	  << "Astrond is a distributed server daemon." << std::endl << std::endl
	  << "-h, --help      Print this help dialog." << std::endl
	  << "-L, --log       Specify a file to write log messages to." << std::endl
	  << "-l, --loglevel  Specify the minimum log level that should be logged;" << std::endl
	  << "                  Security, Error, and Fatal will always be logged;" << std::endl
#ifdef DEBUG_MESSAGES
	  << "                (levels): packet, trace, debug, info, warning, security" << std::endl;
#else
	  << "                (levels): info, warning, security" << std::endl;
#endif

	s.flush();
}
