#include "global.h"
#include "RoleFactory.h"
#include "config/constraints.h"

#include <boost/filesystem.hpp>

#include <cstring>
#include <string>  // std::string
#include <vector>  // std::vector
#include <fstream> // std::ifstream
using namespace std;

static LogCategory mainlog("main", "Main");

static ConfigGroup general_config("general");
static ConfigVariable<vector<string> > dc_files("dc_files", vector<string>(), general_config);
static ConfigVariable<string> eventlogger_addr("eventlogger", "", general_config);

static ConfigList uberdogs_config("uberdogs");
static ConfigVariable<doid_t> uberdog_id("id", INVALID_DO_ID, uberdogs_config);
static ConfigVariable<string> uberdog_class("class", "", uberdogs_config);
static ConfigVariable<bool> uberdog_anon("anonymous", false, uberdogs_config);
static InvalidDoidConstraint id_not_invalid(uberdog_id);
static ReservedDoidConstraint id_not_reserved(uberdog_id);

static void printHelp(ostream &s);


int main(int argc, char *argv[])
{
	string cfg_file;

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
				string llstr(argv[++i]);
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
					cerr << "Unknown log-level \"" << llstr << "\"." << endl;
					printHelp(cerr);
					exit(1);
				}
			}
			else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
			{
				printHelp(cout);
				exit(0);
			}
			else if(argv[i][0] == '-')
			{
				cerr << "Unrecognized option \"" << string(argv[i]) << "\"." << endl;
				printHelp(cerr);
				exit(1);
			}
		}
		if(argv[argc - 1][0] != '-')
		{
			cfg_file = argv[argc - 1];
			// seperate path
			boost::filesystem::path p(cfg_file);
			boost::filesystem::path dir = p.parent_path();
			string filename = p.filename().string();
			string dir_str = dir.string();

			// change directory
			try
			{
				if(!dir_str.empty())
				{
					boost::filesystem::current_path(dir_str);
				}
			}
			catch(const exception &e)
			{
				mainlog.fatal() << "Could not change working directory to config directory.\n";
				exit(1);
			}

			cfg_file = filename; 	
		}
	}

	mainlog.info() << "Loading configuration file..." << endl;


	ifstream file(cfg_file.c_str());
	if(!file.is_open())
	{
		mainlog.fatal() << "Failed to open configuration file.\n";
		return 1;
	}

	if(!g_config->load(file))
	{
		mainlog.fatal() << "Could not parse configuration file!\n";
		return 1;
	}
	file.close();

	if(!ConfigGroup::root().validate(g_config->copy_node()))
	{
		mainlog.fatal() << "Configuration file contains errors.\n";
		return 1;
	}

	vector<string> dc_file_names = dc_files.get_val();
	for(auto it = dc_file_names.begin(); it != dc_file_names.end(); ++it)
	{
		if(!g_dcf->read(*it))
		{
			mainlog.fatal() << "Could not read DC file " << *it << endl;
			return 1;
		}
	}

	// Initialize configured MessageDirector
	MessageDirector::singleton.init_network();
	g_eventsender.init(eventlogger_addr.get_val());

	// Load uberdog metadata from configuration
	ConfigNode udnodes = g_config->copy_node()["uberdogs"];
	if(!udnodes.IsNull())
	{
		for(auto it = udnodes.begin(); it != udnodes.end(); ++it)
		{
			ConfigNode udnode = *it;
			Uberdog ud;
			ud.dcc = g_dcf->get_class_by_name(udnode["class"].as<string>());
			if(!ud.dcc)
			{
				mainlog.fatal() << "DCClass " << udnode["class"].as<string>()
				               << "Does not exist!" << endl;
				exit(1);
			}
			ud.anonymous = udnode["anonymous"].as<bool>();
			g_uberdogs[udnode["id"].as<doid_t>()] = ud;
		}
	}

	// Initialize configured roles
	ConfigNode node = g_config->copy_node();
	node = node["roles"];
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		RoleFactory::singleton().instantiate_role((*it)["type"].as<std::string>(), *it);
	}

	try
	{
		io_service.run();
	}
	catch(exception &e)
	{
		mainlog.fatal() << "Exception from the network io service: "
		                << e.what() << endl;
	}

	return 0;
}

// printHelp outputs the cli help-text to the given stream.
void printHelp(ostream &s)
{
	s << "Usage: astrond [OPTION]... [CONFIG]" << endl
	  << "Astrond is a distributed server daemon." << endl << endl
	  << "-h, --help      Print this help dialog." << endl
	  << "-L, --log       Specify a file to write log messages to." << endl
	  << "-l, --loglevel  Specify the minimum log level that should be logged;" << endl
	  << "                  Security, Error, and Fatal will always be logged;" << endl
#ifdef DEBUG_MESSAGES
	  << "                (levels): packet, trace, debug, info, warning, security" << endl;
#else
	  << "                (levels): info, warning, security" << endl;
#endif

	s.flush();
}
