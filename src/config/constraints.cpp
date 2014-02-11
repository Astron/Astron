#include "constraints.h"

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