State-Server Behavior
---------------------
**Authors**  
Sam "CFSworks" Edwards (08-30-2013)  
Kevin "Kestred" Stenerson (09-04-2013)


### Section 0: Abstract ###

This document describes the messages involved in interacting with a State
Server. A State Server is a component with a single, preconfigured channel.
This channel is used to request the State Server to instantiate new objects.
When an object is instantiated, the object behaves as if it were its own
Message Director participant, and subscribes to its own channel (equal to the
object's ID) to receive object-specific updates. Therefore, the functions of
the State Server's control channel are very narrow compared to the wide range
of control afforded by communicating to an instantiated object directly.


### Section 1: State-Server Control Messages ###

These messages are to be sent directly to the State Server's configured
control channel:

**STATESERVER_OBJECT_CREATE_WITH_REQUIRED(2001)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>)`  
**STATESERVER_OBJECT_CREATE_WITH_REQUIRED_OTHER(2001)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>, <OTHER>)`  
> Create an object on the State Server, specifying its initial location as
> (parent_id, zone_id), its class, and initial field data.


**STATESERVER_DELETE_AI_OBJECTS (2004)** `args(uint64 ai_channel)`  
> Used by an AI Server to inform the State Server that it is going down. The
> State Server will then delete all objects matching the ai_channel.
>
> The AI will typically hang this on its connected MD using ADD_POST_REMOVE, so
> that the message goes out automatically if the AI loses connection unexpectedly.


### Section 2: Distributed Object Control Messages ###

These messages are to be sent to the objects themselves. Objects subscribe to
a channel with their own object ID, and therefore can be reached directly by
using their ID as the channel.


#### Section 2.1: Single Object Accessors ####
These messages provide read and write access to an objects fields. Each accessor
message requires a DistributedObject Id; this means messages can only be sent
to one object at a time.


**STATESERVER_OBJECT_DELETE_RAM(2002)** `args(uint32 do_id)`  
> Delete the object from the State Server; a stored copy on the database is not
> affected, if one exists.
>
> The message is also used to inform others of the delete.  It is broadcast to
> the objects location, send to the managing AI, and also sent to the owner if
> one exists.
> The delete-notification's sender is equal to the original message's sender.


**STATESERVER_OBJECT_GET_FIELD(2010)**  
    `args(uint32 context, uint32 do_id, uint16 field_id)`  
**STATESERVER_OBJECT_GET_FIELD_RESP(2011)**  
    `args(uint32 context, bool success, [uint16 field_id, <VALUE>])`  
> Get the value of a field from a single object.
>
> If the field is unset or invalid, the message returns a failure.


**STATESERVER_OBJECT_GET_FIELDS(2012)**  
    `args(uint32 context, uint32 do_id,
          uint16 field_count, [uint16 field_id]*field_count)`  
**STATESERVER_OBJECT_GET_FIELDS_RESP(2013)**  
    `args(uint32 context, uint8 success,
          uint16 field_count, [uint16 field_id, <VALUE>]*field_count)`  
> Get the value of multiple fields from a single object.
>
> A failure is only returned if one or more of the values are invalid for the
> object's distributed class.  Otherwise, a `field_id, <VALUE>` pair is returned
> for each present field.


**STATESERVER_OBJECT_GET_ALL(2014)** `args(uint32 context, uint32 do_id)`  
**STATESERVER_OBJECT_GET_ALL_RESP(2015)**  
   `args(uint32 context, uint32 do_id,
         uint32 parent_id, uint32 zone_id,
         uint16 dclass_id, <REQUIRED>, <OTHER>)`
> Get the location, class, and fields of a single object.


**STATESERVER_OBJECT_SET_FIELD(2020)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**STATESERVER_OBJECT_SET_FIELDS(2021)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> Set one or more field(s) of a single object.
>
> The message is also used to inform others of the change by the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The change-notification's sender is equal to the original message's sender.
>
> In SET_FIELDS, there are multiple field updates in one message, which will be
> processed as an atomic operation. Note, in the case of field duplicates, the
> last value in the message is used.


**STATESERVER_OBJECT_DELETE_FIELD(2030)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**STATESERVER_OBJECT_DELETE_FIELDS(2031)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> Delete one or more field(s) of a single object.
> Required fields with defaults will be reset.
> Required fields without defaults cannot be deleted.
>
> The message is also used to inform others of the delete by the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The delete-notifications's sender is equal to the original message's sender.
>
> In DELETE_FIELDS, there are multiple field deletes in one message, which will
> be processed as an atomic operation.


#### Section 2.2: Object Visibility Messages ####

**STATESERVER_OBJECT_SET_LOCATION(2040)** `args(uint32 parent_id, uint32 zone_id)` 
**STATESERVER_OBJECT_CHANGING_LOCATION(2041)**  
    `args(uint32 do_id, uint32 new_parent_id, uint32 new_zone_id,
                        uint32 old_parent_id, uint32 old_zone_id)`   
> A set location message moves recieving objects to a new location.
>
> The objects will first broadcast a changing location message to its old location
> channel, as well as its AI channel, and an owner channel if one exists.
>
> Then the objects will broadcast one of the following enter zone messages to
> the new location.


**STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED(2042)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BROADCAST>)`  
**STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER(2043)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BROADCAST>, <OTHER_BROADCAST>)` 
> Used by the object to tell the new location about the object's entry.
>
> The message format is identical to OBJECT_CREATE except that non-broadcast
> fields are not included in the object serialization.  Broadcast fields are the
> only public fields on an object.  Non-broadcast fields are not sent, because
> clients with interest in the object may not be privy to those fields.


**STATESERVER_OBJECT_GET_LOCATION(2044)** `args(uint32 context)`  
**STATESERVER_OBJECT_GET_LOCATION_RESP(2045):**  
    `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id)`  
> Get the location from one or more objects.



# START UNREVISED MESSAGES #



**STATESERVER_OBJECT_GET_ZONE_OBJECTS(2046)**  
    `args(uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_GET_ZONES_OBJECTS(2047)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> Get all child objects in one or more zones from a single object.
>
> Each object will reply with a STATESERVER_OBJECT_ENTER_ZONE message.
> answer with a _WITH_REQUIRED(_OTHER). After all
> objects have answered, the parent will send the corresponding _DONE message.

**STATESERVER_OBJECT_QUERY_ZONE_DONE(????)**
    `args(uint32 parent_id, uint32 zone_id)`
**STATESERVER_OBJECT_QUERY_ZONES_ALL_DONE(2046)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> These are echoes of the above messages. They are sent back to the enquierer after
all objects have announced their existence.



**STATESERVER_OBJECT_SET_AI_CHANNEL(2050)**  
    `args(uint64 ai_channel)`  
**STATESERVER_OBJECT_NOTIFY_MANAGING_AI(2047)**  
    `args(uint32 parent_id, uint64 ai_channel)`  
> _Internally Used_ Broadcast by a parent_id to all of its children (the channel
for this is given by (4030<<32)|parent_id) when its airecv channel changes.
Also sent on demand in response to STATESERVER_OBJECT_QUERY_MANAGING_AI.
**STATESERVER_OBJECT_ENTER_AI_RECV(2067)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 class_id, <REQUIRED>, <OTHER>)`  
**STATESERVER_OBJECT_LEAVING_AI_INTEREST(2033)** `args(uint32 do_id)`  
> Sets the channel for the managing AI. All airecv updates are automatically
forwarded to this channel.  
Note: The managing AI channel can also be set implicitly. If it isn't set
explicitly, it defaults to the AI channel (implicit or explicit) of the
parent.  
> ENTER_AI_RECV tells the new AI Server of the object's arrival.
LEAVING_AI_INTEREST is sent to the old AI Server to notify it of
the object's departure or deletion.


**STATESERVER_OBJECT_SET_OWNER_RECV(2070)** `args(uint64 owner_channel)`
**STATESERVER_OBJECT_CHANGE_OWNER_RECV(2069)**  
    `args(uint32 do_id, uint64 new_owner_channel, uint64 old_owner_channel)`  
**STATESERVER_OBJECT_ENTER_OWNER_RECV(2068):**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id, uint16 dclass_id, <REQUIRED>, <OTHER>)`  
> SET_OWNER sets the channel of the object owner. This is the channel of the Client Agent
connection object where ownrecv messages will be forwarded. Similar to changing zone,
this will generate some traffic:  
CHANGE_OWNER will be sent to the old owner.  
ENTER_OWNER tells the new owner of the object's arrival.



**STATESERVER_OBJECT_QUERY_MANAGING_AI(2083)** `args()`  
> _Internally Used_ The object has just changed parents. It wants to know if its
new parent has a different AI Server channel. This message is sent to the new
parent to request that it resend its AI Server channel.


**STATESERVER_OBJECT_NOTIFY_MANAGING_AI(2047)**  
    `args(uint32 parent_id, uint64 ai_channel)`  
> _Internally Used_ Broadcast by a parent_id to all of its children (the channel
for this is given by (4030<<32)|parent_id) when its airecv channel changes.
Also sent on demand in response to STATESERVER_OBJECT_QUERY_MANAGING_AI.


