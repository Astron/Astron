#pragma once
#include "ConfigVariable.h"
#include "net/address_utils.h"
#include "core/types.h"

bool is_not_invalid_doid(const doid_t& id);
bool is_not_reserved_doid(const doid_t& id);
bool is_not_invalid_channel(const channel_t& c);
bool is_not_reserved_channel(const channel_t& c);
bool is_boolean_keyword(const std::string& str);
bool is_existing_and_readable_file(const std::string& file);

class BooleanValueConstraint : public RawConfigConstraint<bool>
{
  public:
    BooleanValueConstraint(ConfigVariable<bool>& var) :
        RawConfigConstraint(is_boolean_keyword, var,
                            "Boolean value must be either \"true\" or \"false\".")
    {
    }
};
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
class ValidAddressConstraint : public ConfigConstraint<std::string>
{
  public:
    ValidAddressConstraint(ConfigVariable<std::string>& var) :
        ConfigConstraint(is_valid_address, var,
                         "String is not a valid IPv4/IPv6 address or hostname.")
    {
    }
};
class FileAvailableConstraint : public ConfigConstraint<std::string>
{
  public:
    FileAvailableConstraint(ConfigVariable<std::string>& var) :
        ConfigConstraint(is_existing_and_readable_file, var,
                         "File could not be found/opened.")
    {
    }
};
