#include "DatabaseBackend.h"
#include "DatabaseServer.h"
#include "DBBackendFactory.h"
#include <string>
using namespace std;

KeyedConfigGroup db_backend_config("backend", "type", dbserver_config);
ConfigVariable<string> db_backend_type("type", "yaml", db_backend_config);
