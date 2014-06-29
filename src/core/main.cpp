#include "global.h"
#include "RoleFactory.h"
#include "config/constraints.h"

#include "http/HTTPServer.h"

#include "dclass/file/read.h"
#include "dclass/dc/Class.h"
using dclass::Class;

#include <boost/filesystem.hpp>
#include <cstring>
#include <string>  // std::string
#include <vector>  // std::vector
#include <fstream> // std::ifstream

#include <boost/algorithm/string.hpp>

using namespace std;

static LogCategory mainlog("main", "Main");

static ConfigGroup general_config("general");
static ConfigVariable<vector<string> > dc_files("dc_files", vector<string>(), general_config);
static ConfigVariable<string> eventlogger_addr("eventlogger", "", general_config);
static ValidAddressConstraint valid_eventlogger_addr(eventlogger_addr);

static ConfigList uberdogs_config("uberdogs");
static ConfigVariable<doid_t> uberdog_id("id", INVALID_DO_ID, uberdogs_config);
static ConfigVariable<string> uberdog_class("class", "", uberdogs_config);
static ConfigVariable<bool> uberdog_anon("anonymous", false, uberdogs_config);
static InvalidDoidConstraint id_not_invalid(uberdog_id);
static ReservedDoidConstraint id_not_reserved(uberdog_id);
static BooleanValueConstraint anonymous_is_boolean(uberdog_anon);

static ConfigGroup web_config("web");
static ConfigVariable<std::string> web_addr("address", "", web_config);
static ConfigVariable<std::string> web_path("path", "FROZEN", web_config);

static void printHelp(ostream &s);


int main(int argc, char *argv[])
{
	string cfg_file;

	int config_arg_index = -1;
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
			cerr << "Unrecognized option \"" << string(argv[i]) << "\".\n";
			printHelp(cerr);
			exit(1);
		}
		else
		{
			if(config_arg_index != -1)
			{
				cerr << "Recieved additional positional argument \""
				     << string(argv[i]) << "\" but can only accept one."
				     << "\n  First positional: \""
				     << string(argv[config_arg_index]) << "\".\n";
			}
			else
			{
				config_arg_index = i;
			}
		}
	}

	if(config_arg_index != -1)
	{
		string filename = "";
		cfg_file = argv[config_arg_index];

		try
		{
			// seperate path
			boost::filesystem::path p(cfg_file);
			boost::filesystem::path dir = p.parent_path();
			filename = p.filename().string();
			string dir_str = dir.string();

			// change directory
			if(!dir_str.empty())
			{
				boost::filesystem::current_path(dir_str);
			}
		}
		catch(const exception&)
		{
			mainlog.fatal() << "Could not change working directory to config directory.\n";
			exit(1);
		}

		cfg_file = filename; 	
	}

	mainlog.info() << "Loading configuration file...\n";


	ifstream file(cfg_file.c_str());
	if(!file.is_open())
	{
		mainlog.fatal() << "Failed to open configuration file \"" << cfg_file << "\".\n";
		return 1;
	}

	if(!g_config->load(file))
	{
		mainlog.fatal() << "Errors parsing YAML configuration file \"" << cfg_file << "\"!\n";
		return 1;
	}
	file.close();

	if(!ConfigGroup::root().validate(g_config->copy_node()))
	{
		mainlog.fatal() << "Configuration file contains errors.\n";
		return 1;
	}

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
	vector<string> dc_file_names = dc_files.get_val();
	for(auto it = dc_file_names.begin(); it != dc_file_names.end(); ++it)
	{
		bool ok = dclass::append(dcf, *it);
		if(!ok)
		{
			mainlog.fatal() << "Could not read DC file " << *it << endl;
			return 1;
		}
	}
	g_dcf = dcf;

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
	ConfigNode node = g_config->copy_node();
	node = node["roles"];
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		RoleFactory::singleton().instantiate_role((*it)["type"].as<std::string>(), *it);
	}
    
    if(web_addr.get_val().length())
    {
        std::string web_addr_str = web_addr.get_val();
    
        std::vector<std::string> webAddressPort;
        boost::split(webAddressPort, web_addr_str, boost::is_any_of(":"));
    
        string webPort = "80";
        
        if(webAddressPort.size() > 1) 
        {
            webPort = webAddressPort[1];
        }
    
        HTTPServer httpServer (webAddressPort[0], webPort, web_path.get_val());    
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
	s << "Usage:    astrond [options]... [CONFIG_FILE]\n"
	     "\n"
	     "Astrond is a distributed server daemon.\n"
	     "By default Astron looks for a configuration file in the current\n"
	     "working directory as \"astrond.yml\".  A different config file path\n"
	     "can be specified as a positional argument.\n"
	     "\n"
	     "-h, --help      Print this help dialog.\n"
	     "-L, --log       Specify a file to write log messages to.\n"
	     "-l, --loglevel  Specify the minimum log level that should be logged;\n"
	     "                  Security, Error, and Fatal will always be logged;\n"
#ifdef ASTRON_DEBUG_MESSAGES
	     "                (available): packet, trace, debug, info, warning, security\n"
#else
	     "                (available): info, warning, security\n"
	     "                (unavailable): packet, trace, debug\n"
	     "                        [build with -DCMAKE_BUILD_TYPE=Debug]\n"
#endif
         "\n"
         "Example:\n"
         "    astrond /tmp/my_config_file.yaml\n"
         "\n";
	s.flush();
}
