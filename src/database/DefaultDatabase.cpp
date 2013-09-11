#include "Database.h"

ConfigVariable<std::string> storage_folder("storage_folder", "objs/");


void WriteIDFile(RoleConfig &roleconfig, unsigned int doId)
{
	std::ofstream file;
	std::stringstream ss;
	ss << storage_folder.get_rval(roleconfig)  << "id.txt";
	file.open(ss.str());
	file << doId;
	file.close();
}
