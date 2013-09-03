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
#define STATESERVER_ADD_AI_RECV  2045
#define STATESERVER_QUERY_ZONE_OBJECT_ALL_DONE  2046
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

struct DistributedObject
{
	unsigned long long owner;
	unsigned int parentId;
	unsigned int zoneId;
	unsigned int doId;
	DCClass *dcc;
	std::map<DCField*, std::string> fields;
};

std::map<unsigned int, DistributedObject*> distObjs;

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
					for(int i = 0; i < obj->dcc->get_num_inherited_fields(); ++i)
					{
						DCField *field = obj->dcc->get_inherited_field(i);
						if(field->is_required() && !field->as_molecular_field())
						{
							UnpackFieldFromDG(field, dgi, obj->fields[field]);
						}
					}
					distObjs[doId] = obj;
					MessageDirector::singleton.subscribe_channel(this, doId);

					Datagram resp;
					resp.add_uint8(1);
					resp.add_uint64(LOCATION2CHANNEL(parentId, zoneId));
					resp.add_uint64(doId);
					resp.add_uint16(STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
					resp.add_uint32(parentId);
					resp.add_uint32(zoneId);
					resp.add_uint16(dcId);
					resp.add_uint32(doId);
					for(auto it = obj->fields.begin(); it != obj->fields.end(); ++it)
					{
						resp.add_data(it->second);
					}

					MessageDirector::singleton.handle_datagram(&resp, this);
				}
				break;
				case STATESERVER_OBJECT_DELETE_RAM:
				{
					unsigned int doId = dgi.read_uint32();
					DistributedObject *obj = distObjs[doId];
					if(obj)
					{
						unsigned long long loc = LOCATION2CHANNEL(obj->parentId, obj->zoneId);
						Datagram resp;
						resp.add_uint8(1);
						resp.add_uint64(loc);
						resp.add_uint64(doId);
						resp.add_uint16(STATESERVER_OBJECT_DELETE_RAM);
						resp.add_uint32(doId);
						MessageDirector::singleton.handle_datagram(&resp, this);
						distObjs[doId] = NULL;
						delete obj;
					}
				}
				break;
				default:
					gLogger->error() << "StateServer recv'd unknown msgtype: " << MsgType << std::endl;
			}
			return true;
		}
};

RoleFactoryItem<StateServer> ss_fact("stateserver");