p_astron_internal = Proto ("astron_internal", "Astron Internal Protocol (MD)")
p_astron_client = Proto ("astron_client", "Astron Client Protocol (CA)")

astron_md_port = 7199
astron_ca_port = 6667

-- Astron Internal protocol fields

local f_length = ProtoField.uint16("astron_internal.length", "Message length", base.DEC)

local f_recipient = ProtoField.uint64("astron_internal.recipient", "Recipient channel", base.HEX)
local f_sender = ProtoField.uint64("astron_internal.sender", "Sender channel", base.HEX)

local f_msgtype = ProtoField.uint16("astron_internal.msgtype", "Message type", base.DEC)

local f_doid = ProtoField.uint32("astron_internal.doid", "DistributedObject ID", base.DEC)
local f_field = ProtoField.uint16("astron_internal.field", "Field ID", base.DEC)
local f_field_count = ProtoField.uint16("astron_internal.field_count", "Field count", base.DEC)

local f_object_count = ProtoField.uint32("astron_internal.object_count", "Object count", base.DEC)

local f_parent = ProtoField.uint32("astron_internal.parent", "Parent ID", base.DEC)
local f_zone = ProtoField.uint32("astron_internal.zone", "Zone ID", base.DEC)
local f_zone_count = ProtoField.uint16("astron_internal.zone_count", "Zone count", base.DEC)

local f_context = ProtoField.uint32("astron_internal.context", "Request context", base.DEC)

p_astron_internal.fields = {
	f_length, f_recipient, f_sender, f_msgtype,

	f_doid, f_field, f_field_count,

	f_object_count,

	f_parent, f_zone, f_zone_count,

	f_context,
}

-- Astron Client protocol fields

local f_length_client = ProtoField.uint16("astron_client.length", "Message length", base.DEC)
local f_msgtype_client = ProtoField.uint16("astron_client.msgtype", "Message type", base.DEC)

p_astron_client.fields = {
	f_length_client, f_msgtype_client
}

-- Helpers that, maybe someday, will actually get info from a .dc file?
function dclass_by_field (field)
	return "DistributedObject"
end

function dclass_by_index (index)
	return "DistributedObject"
end

function field_by_index (index)
	return "FIELD_" .. index
end

function decode_field (index, buf)
	return "<" .. buf .. ">"
end

local message_table = {
	[1] = {
		name="CLIENT_HELLO",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2] = {
		name="CLIENT_HELLO_RESP",
		dissector=function(buf, root)
			return "" -- No arguments
		end
	},
	[3] = {
		name="CLIENT_DISCONNECT",
		dissector=function(buf, root)
			return "" -- No arguments
		end
	},
	[4] = {
		name="CLIENT_EJECT",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[5] = {
		name="CLIENT_HEARTBEAT",
		dissector=function(buf, root)
			return "" -- No arguments
		end
	},
	[120] = {
		name="CLIENT_OBJECT_SET_FIELD",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[121] = {
		name="CLIENT_OBJECT_SET_FIELDS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[132] = {
		name="CLIENT_OBJECT_LEAVING",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[140] = {
		name="CLIENT_OBJECT_LOCATION",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[142] = {
		name="CLIENT_ENTER_OBJECT_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[143] = {
		name="CLIENT_ENTER_OBJECT_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[172] = {
		name="CLIENT_ENTER_OBJECT_REQUIRED_OWNER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[173] = {
		name="CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[200] = {
		name="CLIENT_ADD_INTEREST",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[201] = {
		name="CLIENT_ADD_INTEREST_MULTIPLE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[203] = {
		name="CLIENT_REMOVE_INTEREST",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[204] = {
		name="CLIENT_DONE_INTEREST_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1000] = {
		name="CLIENTAGENT_SET_STATE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1001] = {
		name="CLIENTAGENT_SET_CLIENT_ID",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1002] = {
		name="CLIENTAGENT_SEND_DATAGRAM",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1004] = {
		name="CLIENTAGENT_EJECT",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1005] = {
		name="CLIENTAGENT_DROP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1006] = {
		name="CLIENTAGENT_GET_NETWORK_ADDRESS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1007] = {
		name="CLIENTAGENT_GET_NETWORK_ADDRESS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1010] = {
		name="CLIENTAGENT_DECLARE_OBJECT",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1011] = {
		name="CLIENTAGENT_UNDECLARE_OBJECT",
		dissector=function(buf, root)
			local doid = buf(0,4):le_uint()
			root:add_le(f_doid, buf(0,4))

			return tostring(doid)
		end
	},
	[1012] = {
		name="CLIENTAGENT_ADD_SESSION_OBJECT",
		dissector=function(buf, root)
			local doid = buf(0,4):le_uint()
			root:add_le(f_doid, buf(0,4))
			
			return tostring(doid)
		end
	},
	[1013] = {
		name="CLIENTAGENT_REMOVE_SESSION_OBJECT",
		dissector=function(buf, root)
			local doid = buf(0,4):le_uint()
			root:add_le(f_doid, buf(0,4))

			return tostring(doid)
		end
	},
	[1014] = {
		name="CLIENTAGENT_SET_FIELDS_SENDABLE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1015] = {
		name="CLIENTAGENT_GET_TLVS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1016] = {
		name="CLIENTAGENT_GET_TLVS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1100] = {
		name="CLIENTAGENT_OPEN_CHANNEL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1101] = {
		name="CLIENTAGENT_CLOSE_CHANNEL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1110] = {
		name="CLIENTAGENT_ADD_POST_REMOVE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1111] = {
		name="CLIENTAGENT_CLEAR_POST_REMOVE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1200] = {
		name="CLIENTAGENT_ADD_INTEREST",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1201] = {
		name="CLIENTAGENT_ADD_INTEREST_MULTIPLE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1203] = {
		name="CLIENTAGENT_REMOVE_INTEREST",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[1204] = {
		name="CLIENTAGENT_DONE_INTEREST_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2000] = {
		name="STATESERVER_CREATE_OBJECT_WITH_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2001] = {
		name="STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2009] = {
		name="STATESERVER_DELETE_AI_OBJECTS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2010] = {
		name="STATESERVER_OBJECT_GET_FIELD",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2011] = {
		name="STATESERVER_OBJECT_GET_FIELD_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2012] = {
		name="STATESERVER_OBJECT_GET_FIELDS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2013] = {
		name="STATESERVER_OBJECT_GET_FIELDS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2014] = {
		name="STATESERVER_OBJECT_GET_ALL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2015] = {
		name="STATESERVER_OBJECT_GET_ALL_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2020] = {
		name="STATESERVER_OBJECT_SET_FIELD",
		dissector=function(buf, root)
			local doid = buf(0,4):le_uint()
			root:add_le(f_doid, buf(0,4))

			local field = buf(4,2):le_uint()
			root:add_le(f_field, buf(0,2), field,
			            "Field: " .. field_by_index(field).."("..field..")")

			local decoded = ""
			if buf:len() > 6 then
				local data = buf(6)
				decoded = decode_field(field, data)
				root:add(data, "Field data: "..decoded)
			end

			return string.format("%s(%d).%s(%s)",
			                     dclass_by_field(field), doid,
			                     field_by_index(field), decoded)
		end
	},
	[2021] = {
		name="STATESERVER_OBJECT_SET_FIELDS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2030] = {
		name="STATESERVER_OBJECT_DELETE_FIELD_RAM",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2031] = {
		name="STATESERVER_OBJECT_DELETE_FIELDS_RAM",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2032] = {
		name="STATESERVER_OBJECT_DELETE_RAM",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2040] = {
		name="STATESERVER_OBJECT_SET_LOCATION",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2041] = {
		name="STATESERVER_OBJECT_CHANGING_LOCATION",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2042] = {
		name="STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2043] = {
		name="STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2044] = {
		name="STATESERVER_OBJECT_GET_LOCATION",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2045] = {
		name="STATESERVER_OBJECT_GET_LOCATION_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2046] = {
		name="STATESERVER_OBJECT_LOCATION_ACK",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2050] = {
		name="STATESERVER_OBJECT_SET_AI",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2051] = {
		name="STATESERVER_OBJECT_CHANGING_AI",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2052] = {
		name="STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2053] = {
		name="STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2054] = {
		name="STATESERVER_OBJECT_GET_AI",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2055] = {
		name="STATESERVER_OBJECT_GET_AI_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2060] = {
		name="STATESERVER_OBJECT_SET_OWNER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2061] = {
		name="STATESERVER_OBJECT_CHANGING_OWNER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2062] = {
		name="STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2063] = {
		name="STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2066] = {
		name="STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2067] = {
		name="STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2100] = {
		name="STATESERVER_OBJECT_GET_ZONE_OBJECTS",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local parent = buf(4,4):le_uint()
			root:add_le(f_parent, buf(4,4))

			local zone = buf(8,4):le_uint()
			root:add_le(f_zone, buf(8,4))

			return string.format("%d: %d, %d", context, parent, zone)
		end
	},
	[2102] = {
		name="STATESERVER_OBJECT_GET_ZONES_OBJECTS",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local parent = buf(4,4):le_uint()
			root:add_le(f_parent, buf(4,4))

			local zone_count = buf(8,2):le_uint()
			local count_item = root:add_le(f_zone_count, buf(8,2))

			local zones = buf(10)
			local zone_strings = {}
			for i = 0, zone_count-1 do
				table.insert(zone_strings, tostring(zones(i*4,4):le_uint()))
				count_item:add_le(f_zone, zones(i*4,4))
			end

			return string.format("%d: %d(%s)", context, parent,
			                     table.concat(zone_strings, "&"))
		end
	},
	[2104] = {
		name="STATESERVER_OBJECT_GET_CHILDREN",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2110] = {
		name="STATESERVER_OBJECT_GET_ZONE_COUNT",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local parent = buf(4,4):le_uint()
			root:add_le(f_parent, buf(4,4))

			local zone = buf(8,4):le_uint()
			root:add_le(f_zone, buf(8,4))

			return string.format("%d: %d, %d", context, parent, zone)
		end
	},
	[2111] = {
		name="STATESERVER_OBJECT_GET_ZONE_COUNT_RESP",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local object_count = buf(4,4):le_uint()
			root:add_le(f_object_count, buf(4,4))

			return string.format("%d: %d", context, object_count)
		end
	},
	[2112] = {
		name="STATESERVER_OBJECT_GET_ZONES_COUNT",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local parent = buf(4,4):le_uint()
			root:add_le(f_parent, buf(4,4))

			local zone_count = buf(8,2):le_uint()
			local count_item = root:add_le(f_zone_count, buf(8,2))

			local zones = buf(10)
			local zone_strings = {}
			for i = 0, zone_count-1 do
				table.insert(zone_strings, tostring(zones(i*4,4):le_uint()))
				count_item:add_le(f_zone, zones(i*4,4))
			end

			return string.format("%d: %d(%s)", context, parent,
			                     table.concat(zone_strings, "&"))
		end
	},
	[2113] = {
		name="STATESERVER_OBJECT_GET_ZONES_COUNT_RESP",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local object_count = buf(4,4):le_uint()
			root:add_le(f_object_count, buf(4,4))

			return string.format("%d: %d", context, object_count)
		end
	},
	[2114] = {
		name="STATESERVER_OBJECT_GET_CHILD_COUNT",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2115] = {
		name="STATESERVER_OBJECT_GET_CHILD_COUNT_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2120] = {
		name="STATESERVER_OBJECT_DELETE_ZONE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2122] = {
		name="STATESERVER_OBJECT_DELETE_ZONES",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2124] = {
		name="STATESERVER_OBJECT_DELETE_CHILDREN",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2125] = {
		name="STATESERVER_GET_ACTIVE_ZONES",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2126] = {
		name="STATESERVER_GET_ACTIVE_ZONES_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2200] = {
		name="DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2201] = {
		name="DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2207] = {
		name="DBSS_OBJECT_GET_ACTIVATED",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2208] = {
		name="DBSS_OBJECT_GET_ACTIVATED_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2230] = {
		name="DBSS_OBJECT_DELETE_FIELD_DISK",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2231] = {
		name="DBSS_OBJECT_DELETE_FIELDS_DISK",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[2232] = {
		name="DBSS_OBJECT_DELETE_DISK",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3000] = {
		name="DBSERVER_CREATE_OBJECT",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3001] = {
		name="DBSERVER_CREATE_OBJECT_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3010] = {
		name="DBSERVER_OBJECT_GET_FIELD",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3011] = {
		name="DBSERVER_OBJECT_GET_FIELD_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3012] = {
		name="DBSERVER_OBJECT_GET_FIELDS",
		dissector=function(buf, root)
			local context = buf(0,4):le_uint()
			root:add_le(f_context, buf(0,4))

			local doid = buf(4,4):le_uint()
			root:add_le(f_doid, buf(4,4))

			local field_count = buf(8,2):le_uint()
			local count_item = root:add_le(f_field_count, buf(8,2))

			local fields = buf(10)
			local field_names = {}
			for i = 0, field_count-1 do
				local field_name = field_by_index(fields(i*2,2):le_uint())
				table.insert(field_names, field_name)

				local field_item = count_item:add_le(f_field, fields(i*2,2))
				field_item:set_text(field_name)
			end

			return string.format("%d[%s]", doid,
			                     table.concat(field_names, "&"))
		end
	},
	[3013] = {
		name="DBSERVER_OBJECT_GET_FIELDS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3014] = {
		name="DBSERVER_OBJECT_GET_ALL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3015] = {
		name="DBSERVER_OBJECT_GET_ALL_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3020] = {
		name="DBSERVER_OBJECT_SET_FIELD",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3021] = {
		name="DBSERVER_OBJECT_SET_FIELDS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3022] = {
		name="DBSERVER_OBJECT_SET_FIELD_IF_EQUALS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3023] = {
		name="DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3024] = {
		name="DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3025] = {
		name="DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3026] = {
		name="DBSERVER_OBJECT_SET_FIELD_IF_EMPTY",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3027] = {
		name="DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3030] = {
		name="DBSERVER_OBJECT_DELETE_FIELD",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3031] = {
		name="DBSERVER_OBJECT_DELETE_FIELDS",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[3032] = {
		name="DBSERVER_OBJECT_DELETE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9000] = {
		name="CONTROL_ADD_CHANNEL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9001] = {
		name="CONTROL_REMOVE_CHANNEL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9002] = {
		name="CONTROL_ADD_RANGE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9003] = {
		name="CONTROL_REMOVE_RANGE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9010] = {
		name="CONTROL_ADD_POST_REMOVE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9011] = {
		name="CONTROL_CLEAR_POST_REMOVES",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9012] = {
		name="CONTROL_SET_CON_NAME",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9013] = {
		name="CONTROL_SET_CON_URL",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
	[9014] = {
		name="CONTROL_LOG_MESSAGE",
		dissector=function(buf, root)
			return "" -- TODO: Dissect
		end
	},
}

function pretty_msgtype (msgtype)
	local msg_entry = message_table[msgtype] or {}

	local msgname = msg_entry.name or "UNKNOWN"
	return string.format("%s(%d)", msgname, msgtype)
end

function pretty_channel (channel)
	local hex = channel:tohex()

	local high = tostring(channel:rshift(32):band(0xFFFFFFFF))
	local low = tostring(channel:band(0xFFFFFFFF))

	return string.format("0x%s(%s, %s)", hex, high, low)
end

function dissect_one (buf, root, packet_descriptions)
	local length = buf(0,2):le_uint()
	local recip_count = buf(2, 1):uint()

	-- Perform several sanity checks to see if we can even begin decode:
	if length < 11 then return 0 end
	if recip_count*8 > buf:len() then return 0 end
	if recip_count == 0 then return 0 end
	if length > 16384 then return 0 end -- Remove this if you have huge messages
	if recip_count > 16 then return 0 end -- Remove this if you have many recipients

	-- Make sure we have enough bytes, ask for more if not:
	if buf:len() < length+2 then return buf:len() - length-2 end
	buf = buf(0, length+2)

	local subtree = root:add(buf, "Message")

	local header_offset = buf:offset()
	local header = subtree:add(buf, "Header")
	header:add_le(f_length, buf(0, 2))
	buf = buf(2)

	local recipients = header:add(buf, "Recipient(s): ")
	local recipient_channel
	for i=0,recip_count-1 do
		local channel_tvb = buf(1+i*8, 8)
		recipients:add_le(f_recipient, channel_tvb)
		if i ~= 0 then recipients:append_text(", ") end
		recipient_channel = channel_tvb:le_uint64()
		recipients:append_text(pretty_channel(recipient_channel))
	end
	recipients:set_len(1 + recip_count*8)
	buf = buf(1 + recip_count*8)

	if(recip_count ~= 1 or tostring(recipient_channel) ~= "1") then
		local sender_channel = buf(0, 8):le_uint64()
		header:add_le(f_sender, buf(0, 8), sender_channel,
		              "Sender channel: " .. pretty_channel(sender_channel))
		buf = buf(8)
	end

	local msgtype = buf(0, 2):le_uint()
	header:add_le(f_msgtype, buf(0, 2), msgtype, "Message type: " .. pretty_msgtype(msgtype))
	if buf:len() > 2 then
		buf = buf(2)
	end

	header:set_len(buf:offset() - header_offset)

	-- Invoke the sub-dissector, if available.
	local full_description = pretty_msgtype(msgtype)
	local dissector = (message_table[msgtype] or {}).dissector
	if dissector ~= nil then
		local description = dissector(buf, subtree)
		if description ~= nil then
			full_description = full_description .. ": " .. description
		end
	else
		subtree:add(buf, "Payload")
	end

	subtree:set_text(full_description)
	table.insert(packet_descriptions, full_description)

	return length + 2
end

-- ASTRON_INTERNAL protocol

function p_astron_internal.dissector (buf, pinfo, root)
	if buf:len() < 2 then return end

	local root = root:add(p_astron_internal, buf())

	local descriptions = {}
	local message_count = 0
	local offset = 0
	while offset < buf:len() do
		local len = dissect_one(buf(offset), root, descriptions)

		if len < 0 then
			pinfo.desegment_offset = offset
			pinfo.desegment_len = -len
			return
		elseif len == 0 then
			return 0
		else
			offset = offset + len
			message_count = message_count + 1
		end
	end

	if message_count > 0 then
		pinfo.cols.protocol = "ASTRON (Internal)"
		pinfo.cols.info = table.concat(descriptions, "; ")
	end

	return offset
end

function p_astron_internal.init()
	local tcp_dissector_table = DissectorTable.get("tcp.port")

	tcp_dissector_table:add(astron_md_port, p_astron_internal)
end

-- ASTRON_CLIENT protocol

function p_astron_client.dissector (buf, pinfo, root)
	if buf:len() < 2 then return end

	local subtree = root:add(p_astron_client, buf())

	local len = buf(0, 2):le_uint()
	local type = buf(2, 2):le_uint()
	
	subtree:add_le(f_length_client, buf(0, 2), len)
	subtree:add_le(f_msgtype_client, buf(2, 2), type, "Message type: " .. pretty_msgtype(type))

	if len > 2 then subtree:add(buf(4), "Payload") end -- TODO: Dissect message payload

	pinfo.cols.protocol = "ASTRON (Client)"
	pinfo.cols.info = pretty_msgtype(type)
end

function p_astron_client.init()
	local tcp_dissector_table = DissectorTable.get("tcp.port")

	tcp_dissector_table:add(astron_ca_port, p_astron_client)
end
