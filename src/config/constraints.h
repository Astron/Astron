#pragma once
#include "ConfigVariable.h"
#include "core/types.h"

bool is_not_invalid_doid(const doid_t& c);
bool is_not_reserved_doid(const doid_t& c);
bool is_not_invalid_channel(const channel_t& c);
bool is_not_reserved_channel(const channel_t& c);

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
