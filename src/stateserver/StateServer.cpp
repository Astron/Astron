#include "util/Role.h"
#include "core/RoleFactory.h"
#include "core/global.h"
#include <map>
#include <hash_map>
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"

#pragma region definitions
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED 2001
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER 2003
#define STATESERVER_OBJECT_UPDATE_FIELD  2004
#define STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE  2005
#define STATESERVER_OBJECT_DELETE_RAM  2007
#define STATESERVER_OBJECT_SET_ZONE  2008
#define STATESERVER_OBJECT_CHANGE_ZONE  2009
#define STATESERVER_OBJECT_NOTFOUND  2015
#define STATESERVER_QUERY_OBJECT_ALL  2020
#define STATESERVER_QUERY_ZONE_OBJECT_ALL  2021
#define STATESERVER_OBJECT_LOCATE  2022
#define STATESERVER_OBJECT_LOCATE_RESP  2023
#define STATESERVER_OBJECT_QUERY_FIELD  2024
#define STATESERVER_QUERY_OBJECT_ALL_RESP  2030
#define STATESERVER_OBJECT_LEAVING_AI_INTEREST  2033
#define STATESERVER_OBJECT_SET_AI_CHANNEL  2045
#define STATESERVER_QUERY_ZONE_OBJECT_ALL_DONE  2046
#define STATESERVER_OBJECT_NOTIFY_MANAGING_AI 2047
#define STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT  2050
#define STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT  2051
#define STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT_RESP  2052
#define STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT_RESP  2053
#define STATESERVER_OBJECT_DELETE_DISK  2060
#define STATESERVER_SHARD_REST  2061
#define STATESERVER_OBJECT_QUERY_FIELD_RESP  2062
#define STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED  2065
#define STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER  2066
#define STATESERVER_OBJECT_ENTER_AI_RECV  2067
#define STATESERVER_OBJECT_ENTER_OWNER_RECV  2068
#define STATESERVER_OBJECT_CHANGE_OWNER_RECV  2069
#define STATESERVER_OBJECT_SET_OWNER_RECV  2070
#define STATESERVER_OBJECT_QUERY_FIELDS  2080
#define STATESERVER_OBJECT_QUERY_FIELDS_RESP  2081
#define STATESERVER_OBJECT_QUERY_FIELDS_STRING  2082
#define STATESERVER_OBJECT_QUERY_MANAGING_AI  2083
#define STATESERVER_BOUNCE_MESSAGE  2086
#define STATESERVER_QUERY_OBJECT_CHILDREN_LOCAL  2087
#define STATESERVER_QUERY_OBJECT_CHILDREN_LOCAL_DONE  2089
#define STATESERVER_QUERY_OBJECT_CHILDREN_RESP  2087

#define LOCATION2CHANNEL(p, z) unsigned long long(p)<<32|unsigned long long(z)
#pragma endregion

void UnpackFieldFromDG(DCPackerInterface *field, DatagramIterator &dgi, std::string &str)
{
	if(field->has_fixed_byte_size())
	{
		str += dgi.read_data(field->get_fixed_byte_size());
	}
	else if(field->get_num_length_bytes() > 0)
	{
		unsigned int length = field->get_num_length_bytes();
		switch(length)
		{
		case 2:
		{
			unsigned short l = dgi.read_uint16();
			str += std::string((char*)&l, 2);
			length = l;
		}
		break;
		case 4:
		{
			unsigned int l = dgi.read_uint32();
			str += std::string((char*)&l, 4);
			length = l;
		}
		break;
		break;
		}
		str += dgi.read_data(length);
	}
	else
	{
		unsigned int nNested = field->get_num_nested_fields();
		for(unsigned int i = 0; i < nNested; ++i)
		{
			UnpackFieldFromDG(field->get_nested_field(i), dgi, str);
		}
	}
}

struct DistributedObject;

std::map<unsigned int, DistributedObject*> distObjs;

struct DistributedObject : public MDParticipantInterface
{
	unsigned long long owner;
	unsigned int parentId;
	unsigned int zoneId;
	unsigned int doId;
	DCClass *dcc;
	std::map<DCField*, std::string> fields;
	unsigned long long aiChannel;
	bool aiExplicitlySet;

	std::string generate_required_data()
	{
		Datagram dg;
		dg.add_uint32(parentId);
		dg.add_uint32(zoneId);
		dg.add_uint16(dcc->get_number());
		dg.add_uint32(doId);
		for(auto it = fields.begin(); it != fields.end(); ++it)
		{
			if(it->first->is_required())
			{
				dg.add_data(it->second);
			}
		}
		return std::string(dg.get_data(), dg.get_buf_end());
	}

	std::string generate_other_data()
	{
		unsigned int nFields = 0;
		Datagram fieldData;
		for(auto it = fields.begin(); it != fields.end(); ++it)
		{
			if(it->first->is_ram() && !it->first->is_required())
			{
				nFields++;
				fieldData.add_string(it->first->get_name());
				fieldData.add_data(it->second);
			}
		}
		Datagram dg;
		dg.add_uint16(nFields);
		dg.add_data(std::string(fieldData.get_data(), fieldData.get_buf_end()));
		return std::string(dg.get_data(), dg.get_buf_end());
	}

	virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
	{
		unsigned long long sender = dgi.read_uint64();
		unsigned short MsgType = dgi.read_uint16();
		switch(MsgType)
		{
			case STATESERVER_OBJECT_DELETE_RAM:
			{
				unsigned int p_doId = dgi.read_uint32();
				if(p_doId == doId)
				{
					unsigned long long loc = LOCATION2CHANNEL(parentId, zoneId);
					Datagram resp;
					resp.add_uint8(1);
					resp.add_uint64(loc);
					resp.add_uint64(doId);
					resp.add_uint16(STATESERVER_OBJECT_DELETE_RAM);
					resp.add_uint32(doId);
					MessageDirector::singleton.handle_datagram(&resp, this);
					distObjs[doId] = NULL;
					gLogger->debug() << "DELETING THIS " << doId << std::endl;
					delete this;
				}
			}
			break;
			case STATESERVER_OBJECT_UPDATE_FIELD:
			{
				unsigned int r_doId = dgi.read_uint32();
				unsigned int fieldId = dgi.read_uint16();
				if(doId == r_doId)
				{
					std::string data;
					DCField *field = dcc->get_field_by_index(fieldId);
					UnpackFieldFromDG(field, dgi, data);
					if(field->is_required() || field->is_ram())
					{
						fields[field] = data;
					}
					Datagram resp;
					if(field->is_broadcast() && field->is_airecv())
					{
						resp.add_uint8(2);
						resp.add_uint64(LOCATION2CHANNEL(parentId, zoneId));
						resp.add_uint64(aiChannel);
					}
					else if(field->is_broadcast())
					{
						resp.add_uint8(1);
						resp.add_uint64(LOCATION2CHANNEL(parentId, zoneId));
					}
					else if(field->is_airecv())
					{
						resp.add_uint8(1);
						resp.add_uint64(aiChannel);
					}
					if(field->is_broadcast() | field->is_airecv())
					{
						resp.add_uint64(sender);
						resp.add_uint16(STATESERVER_OBJECT_UPDATE_FIELD);
						resp.add_uint32(doId);
						resp.add_uint16(fieldId);
						resp.add_data(data);
						MessageDirector::singleton.handle_datagram(&resp, this);
					}
				}
			}
			break;
			case STATESERVER_OBJECT_NOTIFY_MANAGING_AI:
				gLogger->debug() << "STATESERVER_OBJECT_NOTIFY_MANAGING_AI " << parentId << std::endl;
				if(aiExplicitlySet)
					break;
			case STATESERVER_OBJECT_SET_AI_CHANNEL:
			{
				gLogger->debug() << "STATESERVER_OBJECT_SET_AI_CHANNEL " << doId << std::endl;
				unsigned int r_doId = dgi.read_uint32();
				gLogger->debug() << "r_doId " << r_doId << std::endl;
				unsigned long long r_aiChannel = dgi.read_uint64();
				if(aiChannel == r_aiChannel)
					break;
				if((MsgType == STATESERVER_OBJECT_NOTIFY_MANAGING_AI && r_doId == parentId) || doId == r_doId)
				{
					if(aiChannel)
					{
						Datagram resp;
						resp.add_uint8(1);
						resp.add_uint64(aiChannel);
						resp.add_uint64(doId);
						resp.add_uint16(STATESERVER_OBJECT_LEAVING_AI_INTEREST);
						resp.add_uint32(doId);
						gLogger->debug() << doId << " LEAVING AI INTEREST" << std::endl;
						MessageDirector::singleton.handle_datagram(&resp, this);
					}

					aiChannel = r_aiChannel;
					aiExplicitlySet = MsgType == STATESERVER_OBJECT_SET_AI_CHANNEL;

					Datagram resp;
					resp.add_uint8(1);
					resp.add_uint64(aiChannel);
					resp.add_uint64(doId);
					resp.add_uint16(STATESERVER_OBJECT_ENTER_AI_RECV);
					resp.add_data(generate_required_data());
					resp.add_data(generate_other_data());
					MessageDirector::singleton.handle_datagram(&resp, this);
					gLogger->debug() << "Sending STATESERVER_OBJECT_ENTER_AI_RECV to " << aiChannel
						<< " from " << doId << std::endl;

					Datagram resp2;
					resp2.add_uint8(1);
					resp2.add_uint64(LOCATION2CHANNEL(4030, doId));
					resp2.add_uint64(doId);
					resp2.add_uint16(STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
					resp2.add_uint32(doId);
					resp2.add_uint64(aiChannel);
					MessageDirector::singleton.handle_datagram(&resp2, this);
				}
			}
			break;
			case STATESERVER_OBJECT_QUERY_MANAGING_AI:
			{
				gLogger->debug() << "STATESERVER_OBJECT_QUERY_MANAGING_AI" << std::endl;
				Datagram resp;
				resp.add_uint8(1);
				resp.add_uint64(sender);
				resp.add_uint64(doId);
				resp.add_uint16(STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
				resp.add_uint32(doId);
				resp.add_uint64(aiChannel);
				MessageDirector::singleton.handle_datagram(&resp, this);
			}
			break;
			case STATESERVER_OBJECT_SET_ZONE:
			{
				unsigned int oParentId = parentId, oZoneId = zoneId;
				parentId = dgi.read_uint32();
				zoneId = dgi.read_uint32();

				MessageDirector::singleton.unsubscribe_channel(this, LOCATION2CHANNEL(4030, oParentId));
				MessageDirector::singleton.subscribe_channel(this, LOCATION2CHANNEL(4030, parentId));

				if(aiChannel)
				{
					Datagram resp2;
					resp2.add_uint8(1);
					resp2.add_uint64(aiChannel);
					resp2.add_uint64(sender);
					resp2.add_uint16(STATESERVER_OBJECT_CHANGE_ZONE);
					resp2.add_uint32(doId);
					resp2.add_uint32(parentId);
					resp2.add_uint32(zoneId);
					resp2.add_uint32(oParentId);
					resp2.add_uint32(oZoneId);
					gLogger->debug() << "Sending STATESERVER_OBJECT_CHANGE_ZONE to " << aiChannel << " "
						<< doId << std::endl;
					MessageDirector::singleton.handle_datagram(&resp2, this);
				}

				Datagram resp;
				resp.add_uint8(1);
				resp.add_uint64(parentId);
				resp.add_uint64(doId);
				resp.add_uint16(STATESERVER_OBJECT_QUERY_MANAGING_AI);
				MessageDirector::singleton.handle_datagram(&resp, this);
			}
			break;
			default:
				gLogger->warning() << "DistributedObject recv'd unkonw MsgType " << MsgType << std::endl;
		}
		return true;
	}
};

ConfigVariable<unsigned long long> cfg_channel("control", 0);

class StateServer : public Role
{
	public:
		StateServer(RoleConfig roleconfig) : Role(roleconfig)
		{
			MessageDirector::singleton.subscribe_channel(this, cfg_channel.get_rval(m_roleconfig));
		}

		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
		{
			unsigned long long sender = dgi.read_uint64();
			unsigned short MsgType = dgi.read_uint16();
			switch(MsgType)
			{
				case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED:
				{
					unsigned int parentId = dgi.read_uint32();
					unsigned int zoneId = dgi.read_uint32();
					unsigned short dcId = dgi.read_uint16();
					unsigned int doId = dgi.read_uint32();

					DistributedObject *obj = new DistributedObject;
					obj->doId = doId;
					obj->parentId = parentId;
					obj->zoneId = zoneId;
					obj->owner = sender;
					obj->dcc = gDCF->get_class(dcId);
					obj->aiChannel = 0;
					obj->aiExplicitlySet = false;

					for(int i = 0; i < obj->dcc->get_num_inherited_fields(); ++i)
					{
						DCField *field = obj->dcc->get_inherited_field(i);
						if(field->is_required() && !field->as_molecular_field())
						{
							UnpackFieldFromDG(field, dgi, obj->fields[field]);
						}
					}
					distObjs[doId] = obj;
					MessageDirector::singleton.subscribe_channel(obj, doId);

					Datagram resp;
					resp.add_uint8(1);
					resp.add_uint64(LOCATION2CHANNEL(parentId, zoneId));
					resp.add_uint64(doId);
					resp.add_uint16(STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
					resp.add_data(obj->generate_required_data());

					MessageDirector::singleton.handle_datagram(&resp, this);

					Datagram resp2;
					resp2.add_uint8(1);
					resp2.add_uint64(parentId);
					resp2.add_uint64(doId);
					resp2.add_uint16(STATESERVER_OBJECT_QUERY_MANAGING_AI);
					gLogger->debug() << "sending STATESERVER_OBJECT_QUERY_MANAGING_AI to "
						<< parentId << " from " << doId << std::endl;
					MessageDirector::singleton.handle_datagram(&resp2, this);
				}
				break;
				default:
					gLogger->error() << "StateServer recv'd unknown msgtype: " << MsgType << std::endl;
			}
			return true;
		}
};

RoleFactoryItem<StateServer> ss_fact("stateserver");