#include "ConfigVariable.h"

bool is_not_invalid_doid(const doid_t& c)
{
	return c != INVALID_DO_ID;
}
bool is_not_reserved_doid(const doid_t& c)
{
	return (c < 1) || (c > 999); 
}

class InvalidDoidConstraint : public ConfigConstraint<doid_t>
{
	public:
		InvalidDoidConstraint(ConfigVariable<doid_t>& var) :
			ConfigConstraint(is_not_invalid_doid, var,
				"Doid value cannot be INVALID_DOID (0).")
		{
		}
};
class ReservedDoidConstraint : public ConfigConstraint<doid_t>
{
	public:
		ReservedDoidConstraint(ConfigVariable<doid_t>& var) :
			ConfigConstraint(is_not_reserved_doid, var,
				"Doid value cannot be reserved doid (1 - 999).")
		{
		}
};

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


class InvalidChannelConstraint : public ConfigConstraint<channel_t>
{
	public:
		InvalidChannelConstraint(ConfigVariable<channel_t>& var) :
			ConfigConstraint(is_not_invalid_channel, var,
				"Channel value cannot be INVALID_CHANNEL (0).")
		{
		}
};
class ReservedChannelConstraint : public ConfigConstraint<channel_t>
{
	public:
		ReservedChannelConstraint(ConfigVariable<channel_t>& var) :
			ConfigConstraint(is_not_reserved_channel, var,
				"Channel value cannot be in a reserved channel range.")
		{
		}
};
