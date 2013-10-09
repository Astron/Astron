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

**STATESERVER_OBJECT_CREATE_WITH_REQUIRED(2000)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>)`  
**STATESERVER_OBJECT_CREATE_WITH_REQUIRED_OTHER(2001)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>, <OTHER>)`  
> Create an object on the State Server, specifying its initial location as
> (parent_id, zone_id), its class, and initial field data.


**STATESERVER_DELETE_AI_OBJECTS (2009)** `args(uint64 ai_channel)`  
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


**STATESERVER_OBJECT_DELETE_RAM(2008)** `args(uint32 do_id)`  
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
> A set location message moves receiving objects to a new location.
>
> The objects will first broadcast a changing location message to its old location
> channel, as well as its AI channel, and an owner channel if one exists.
>
> Then the objects will broadcast one of the following enter location messages
> to the new location.


**STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED(2042)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST>)`  
**STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER(2043)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST>, <OTHER_BCAST>)` 
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


**STATESERVER_OBJECT_SET_AI(2050)** `args(uint64 ai_channel)`  
**STATESERVER_OBJECT_CHANGING_AI(2051)**  
    `args(uint32 doid_id, uint64 new_ai_channel, uint64 old_ai_channel)`  
> A set AI message moves receiving objects to a new AI.
>
> The objects will first broadcast a changing AI message to its old AI channel,
> as well as to all of its children on the parent message channel (1 << 32|parent_id).
> Children whose AI channels have not been explicitly set will process the message
> as if they had recieved a set AI message with ai_channel equal to the new AI;
> their AI channel is implicitly set to that of their parent.
>
> Then the objects will send one of the following enter AI messages to the new AI.


**STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED(2052)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 class_id, <REQUIRED>)`  
**STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER(2053)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 class_id, <REQUIRED>, <OTHER>)`  
> Used by the object to tell the new AI about the objects' entry.
>
> The message format is identical to OBJECT_CREATE.


**STATESERVER_OBJECT_GET_AI(2054)** `args(uint32 context)`  
**STATESERVER_OBJECT_GET_AI_RESP(2055):**  
    `args(uint32 context, uint32 do_id, uint64 ai_channel)`  
> Get the AI from one or more objects.
>
> This message is sent automatically by an object that has just changed parents
> to check if the new parent has a different AI channel. If the object's AI
> channel has not been explicitly set, it will act as if it had recieved a set
> AI message; sending out its own changing and entering AI messages.


**STATESERVER_OBJECT_SET_OWNER(2070)** `args(uint64 owner_channel)`
**STATESERVER_OBJECT_CHANGING_OWNER(2071)**  
    `args(uint32 do_id, uint64 new_owner_channel, uint64 old_owner_channel)`  
> A set owner message moves receiving object to a new owner.
>
> The objects will first send a change owner message to its older owner.
>
> Then the objects will send one of the following enter owner messages to the
> new owner.


**STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED(2072):**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST_OR_OWNRECV>)`  
**STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER(2073):**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST_OR_OWNRECV>, <OTHER_BCAST_OR_OWNRECV>)`  
> Used by the object to tell the new owner about the object's entry.
>
> The message format is identical to OBJECT_CREATE except that only fields with
> the broadcast or ownrecv keyword are included in the object serialization.
> Other fields are not sent, because the owner may not be privy to those fields.


#### Section 2.3: Client Interest Methods ####



__ STILL UNDER BRAINSTORMING __



**STATESERVER_OBJECT_GET_ZONE_OBJECTS(2100)**  
    `args(uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_GET_ZONES_OBJECTS(2101)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> Get all child objects in one or more zones from a single object.
>
> The parent will reply with a ZONE_OBJECT_COUNT or ZONES_OBJECT_COUNT message.
>
> Each object will reply with a STATESERVER_OBJECT_ENTER_LOCATION message.


**STATESERVER_OBJECT_ZONE_OBJECT_COUNT(2102)**
    `args(uint32 parent_id, uint32 zone_id)`
**STATESERVER_OBJECT_ZONES_OBJECT_COUNT(2102)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> These are echoes of the above messages. They are sent back to the enquierer after
all objects have announced their existence.
