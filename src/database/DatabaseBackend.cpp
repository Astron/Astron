#include "DatabaseBackend.h"
#include "DatabaseServer.h"
#include "DBBackendFactory.h"
#include <string>
using namespace std;

ConfigGroup db_backend_config("backend", dbserver_config);
ConfigVariable<string> db_backend_type("type", "yaml", db_backend_config);

bool have_backend(const string& backend)
{
    return DBBackendFactory::singleton().has_backend(backend);
}
ConfigConstraint<string> db_backend_exists(have_backend, db_backend_type,
        "No database backend exists for the given backend type.");
