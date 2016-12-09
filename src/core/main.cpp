#include "global.h"
#include "shutdown.h"
#include "RoleFactory.h"
#include "config/constraints.h"
#include "dclass/file/read.h"
#include "dclass/dc/Class.h"
using dclass::Class;

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
static ValidAddressConstraint valid_eventlogger_addr(eventlogger_addr);

static ConfigList uberdogs_config("uberdogs");
static ConfigVariable<doid_t> uberdog_id("id", INVALID_DO_ID, uberdogs_config);
static ConfigVariable<string> uberdog_class("class", "", uberdogs_config);
static ConfigVariable<bool> uberdog_anon("anonymous", false, uberdogs_config);
static InvalidDoidConstraint id_not_invalid(uberdog_id);
static ReservedDoidConstraint id_not_reserved(uberdog_id);
static BooleanValueConstraint anonymous_is_boolean(uberdog_anon);

static void printHelp(ostream &s);
static void printCompiledOptions(ostream &s);

int main(int argc, char *argv[])
{
    string cfg_file;

#ifdef _WIN32
    bool prettyPrint = false;
#else
    bool prettyPrint = true;
#endif

    int config_arg_index = -1;
    cfg_file = "astrond.yml";
    LogSeverity sev = g_logger->get_min_severity();
    for(int i = 1; i < argc; i++) {
        if((strcmp(argv[i], "--log") == 0 || strcmp(argv[i], "-L") == 0) && i + 1 < argc) {
            g_logger.reset(new Logger(argv[++i], sev));
        } else if((strcmp(argv[i], "--loglevel") == 0 || strcmp(argv[i], "-l") == 0) && i + 1 < argc) {
            string llstr(argv[++i]);
            if(llstr == "packet") {
                sev = LSEVERITY_PACKET;
                g_logger->set_min_severity(sev);
            } else if(llstr == "trace") {
                sev = LSEVERITY_TRACE;
                g_logger->set_min_severity(sev);
            } else if(llstr == "debug") {
                sev = LSEVERITY_DEBUG;
                g_logger->set_min_severity(sev);
            } else if(llstr == "info") {
                sev = LSEVERITY_INFO;
                g_logger->set_min_severity(sev);
            } else if(llstr == "warning") {
                sev = LSEVERITY_INFO;
                g_logger->set_min_severity(sev);
            } else if(llstr == "security") {
                sev = LSEVERITY_SECURITY;
                g_logger->set_min_severity(sev);
            } else if(llstr != "error" && llstr != "fatal") {
                cerr << "Unknown log-level \"" << llstr << "\"." << endl;
                printHelp(cerr);
                return 1;
            }
        } else if(strcmp(argv[i], "--pretty") == 0 || strcmp(argv[i], "-p") == 0) {
            prettyPrint = true;
        } else if(strcmp(argv[i], "--boring") == 0 || strcmp(argv[i], "-b") == 0) {
            prettyPrint = false;
        } else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printHelp(cout);
            return 0;
        } else if(strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            cout << "A Server Technology for Realtime Object Networking (Astron)\n"
                 "http://github.com/astron/astron\n"

#ifdef GIT_SHA1
                 "Revision: " << GIT_SHA1 << "\n"
#else
                 "Revision: NOT-IN-GIT\n"
#endif
                 "Compiled at " << __TIME__ << " on " << __DATE__ << endl;

            printCompiledOptions(cout);

            return 0;
        } else if(argv[i][0] == '-') {
            cerr << "Unrecognized option \"" << string(argv[i]) << "\".\n";
            printHelp(cerr);
            return 1;
        } else {
            if(config_arg_index != -1) {
                cerr << "Recieved additional positional argument \""
                     << string(argv[i]) << "\" but can only accept one."
                     << "\n  First positional: \""
                     << string(argv[config_arg_index]) << "\".\n";
            } else {
                config_arg_index = i;
            }
        }
    }

    g_logger->set_color_enabled(prettyPrint);

    if(config_arg_index != -1) {
        string filename = "";
        cfg_file = argv[config_arg_index];

        try {
            // seperate path
            boost::filesystem::path p(cfg_file);
            boost::filesystem::path dir = p.parent_path();
            filename = p.filename().string();
            string dir_str = dir.string();

            // change directory
            if(!dir_str.empty()) {
                boost::filesystem::current_path(dir_str);
            }
        } catch(const boost::filesystem::filesystem_error&) {
            mainlog.fatal() << "Could not change working directory to config directory.\n";
            return 1;
        }

        cfg_file = filename;
    }

    mainlog.info() << "Loading configuration file...\n";


    ifstream file(cfg_file.c_str());
    if(!file.is_open()) {
        mainlog.fatal() << "Failed to open configuration file \"" << cfg_file << "\".\n";
        return 1;
    }

    if(!g_config->load(file)) {
        mainlog.fatal() << "Errors parsing YAML configuration file \"" << cfg_file << "\"!\n";
        return 1;
    }
    file.close();

    if(!ConfigGroup::root().validate(g_config->copy_node())) {
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
    for(auto it = dc_file_names.begin(); it != dc_file_names.end(); ++it) {
        bool ok = dclass::append(dcf, *it);
        if(!ok) {
            mainlog.fatal() << "Could not read DC file " << *it << endl;
            return 1;
        }
    }
    g_dcf = dcf;

    // Now hook up our speciailize signal handler
    astron_handle_signals();

    try {
        // Initialize configured MessageDirector
        MessageDirector::singleton.init_network();
        g_eventsender.init(eventlogger_addr.get_val());

        // Load uberdog metadata from configuration
        ConfigNode udnodes = g_config->copy_node()["uberdogs"];
        if(!udnodes.IsNull()) {
            for(auto it = udnodes.begin(); it != udnodes.end(); ++it) {
                ConfigNode udnode = *it;
                Uberdog ud;

                // Get the uberdog's class
                const Class* dcc = g_dcf->get_class_by_name(udnode["class"].as<std::string>());
                if(!dcc) {
                    // Make sure it exists
                    mainlog.fatal() << "For uberdog " << udnode["id"].as<doid_t>()
                                    << " Distributed class " << udnode["class"].as<std::string>()
                                    << " does not exist!" << std::endl;
                    return 1;
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
        for(auto it = node.begin(); it != node.end(); ++it) {
            RoleFactory::singleton().instantiate_role((*it)["type"].as<std::string>(), *it);
        }
    }
    // This exception is propogated if astron_shutdown is called
    catch(const ShutdownException& e) {
        return e.exit_code();
    }

    // Run the main event loop
    int exit_code = 0;
    try {
        io_service.run();
    }

    // This exception is propogated if astron_shutdown is called
    catch(const ShutdownException& e) {
        exit_code = e.exit_code();
    }

    // Catch any other exception that propogates
    catch(const exception &e) {
        mainlog.fatal() << "Uncaught exception from the main event loop: "
                        << e.what() << endl;
        return 1;
    }

    return exit_code;
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
      "-v, --version   Print Version, Module and Compilation Information\n"
      "-L, --log       Specify a file to write log messages to.\n"
      "-p, --pretty    Enables colored pretty printing. \n"
      "-b, --boring    Disables colored pretty printing. \n"
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

void printCompiledOptions(ostream &s)
{
    s << "Compilation options: "

      //If on, datagrams and dclass fields will use 32-bit length tags instead of 16-bit.
#ifdef ASTRON_32BIT_DATAGRAMS
      "32-bit length tag Datagrams, "
#else
      "16-bit length tag Datagrams, "
#endif

      //If on, channels will be 128-bit and doids and zones will be 64-bit (instead of 64/32).
#ifdef ASTRON_128BIT_CHANNELS
      "128-bit channel space, 64-bit distributed object id's, 64-bit zones"
#else
      "64-bit channel space, 32-bit distributed object id's, 32-bit zones"
#endif
      << "\n";

    //Now print what parts are compiled in.
    s << "Components: "
#ifdef BUILD_STATESERVER
      "State Server "
#ifdef BUILD_STATESERVER_DBSS
      "(With DBSS capablities)"
#endif //End DBSS
      ", "
#endif //End SS

#ifdef BUILD_EVENTLOGGER
      "Event Logger, "
#endif

#ifdef BUILD_CLIENTAGENT
      "Client Agent, "
#endif

#ifdef BUILD_DBSERVER
      "Database "

#ifdef BUILD_DB_YAML
      "(With YAML Support) "
#endif //End DB_YAML

#ifdef BUILD_DB_SQL
      "(With SQL DB Support) "
#endif //End DB_SQL

#endif //End DBSERVER
      "\n";



}

// http://stackoverflow.com/a/34207323
#if defined(_MSC_VER) && (_MSC_VER < 1900)
struct VS2013_threading_fix
{
	VS2013_threading_fix()
	{
		_Cnd_do_broadcast_at_thread_exit();
	}
} threading_fix;
#endif
