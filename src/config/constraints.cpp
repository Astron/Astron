#include "constraints.h"
#include "util/filesystem.h"
#include <algorithm>
using namespace std;

bool is_not_invalid_doid(const doid_t& c)
{
    return c != INVALID_DO_ID;
}
bool is_not_reserved_doid(const doid_t& c)
{
    return (c < 1) || (c > 999);
}
bool is_not_invalid_channel(const channel_t& c)
{
    return c != INVALID_CHANNEL;
}
bool is_not_reserved_channel(const channel_t& c)
{
    return (c < 1)
           || ((c > 999) && (c < (channel_t(1) << ZONE_BITS)))
           || (c > channel_t(999) << ZONE_BITS);
}
bool is_boolean_keyword(const string& str)
{
    string lower = str;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return (lower == "false") || (lower == "true");
}

bool is_existing_and_readable_file(const std::string& file)
{
    return fs::file_exists(file) && fs::is_readable(file);
}
