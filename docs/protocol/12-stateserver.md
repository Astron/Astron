State-Server Behavior
---------------------
**Authors**  
Sam "CFSworks" Edwards (08-30-2013)  
Kevin "Kestred" Stenerson (09-04-2013)
Jeremy "jjkoletar" Koletar (10-09-2014)


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

**STATESERVER_CREATE_OBJECT_WITH_REQUIRED(2000)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>)`  
**STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER(2001)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED>, <OTHER>)`  
> Create an object on the State Server, specifying its initial location as
> (parent_id, zone_id), its class, and initial field data.
>
> The object then broadcasts an ENTER_LOCATION message to its location channel,
> and sends a CHANGING_ZONE with old location (0,0) to its parent (if it has one).
>
> Additionally, the object sends a GET_LOCATION to its children over the parent
> messages channel (1 << 32|parent_id) with context 1001 (STATESERVER_CONTEXT_WAKE_CHILDREN).


**STATESERVER_DELETE_AI_OBJECTS(2009)** `args(uint64 ai_channel)`  
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
          [uint16 field_count], [uint16 field_id, <VALUE>]*field_count)`  
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
    `args(uint32 do_id, uint16 field_count, [uint16 field_id, <VALUE>]*field_count)`  
> Set one or more field(s) of a single object.
>
> The message is also used to inform others of the change by the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The broadcast's "sender" is set to the original message's sender.
>
> In SET_FIELDS, there are multiple field updates in one message, which will be
> processed as an atomic operation. Note, in the case of field duplicates, the
> last value in the message is used.


**STATESERVER_OBJECT_DELETE_FIELD_RAM(2030)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**STATESERVER_OBJECT_DELETE_FIELDS_RAM(2031)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> Delete one or more field(s) of a single object.
> Required fields with defaults will be reset.
> Required fields without defaults cannot be deleted.
>
> The message is also used to inform others of the delete by the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The broadcast's "sender" is set to the original message's sender.
>
> In DELETE_FIELDS, there are multiple field deletes in one message, which will
> be processed as an atomic operation.


**STATESERVER_OBJECT_DELETE_RAM(2032)** `args(uint32 do_id)`  
> Delete the object from the State Server; a stored copy on the database is not
> affected, if one exists.
>
> The message is also used to inform others of the delete.
>  - If it has a parent, it is broadcast to the object's location.
>  - If it has a managing AI, it is broadcast to the object' AI.
>  - If it has an owner, it is broadcast to the object's owner.
> The broadcast's "sender" is set to the original message's sender.
>
> If the object has any children, it sends a STATESERVER_OBJECT_DELETE_CHILDREN
> message over the parent messages channel (1 << 32|parent_id).
>
> If the object received an explicit DELETE_RAM and not an implicit delete (such
> as a DELETE_CHILDREN from a parent), it sends a STATSERVER_OBJECT_CHANGING_LOCATION
> to its parent object with INVALID_PARENT and INVALID_ZONE as the new location.
>



#### Section 2.2: Object Visibility Messages ####

**STATESERVER_OBJECT_SET_LOCATION(2040)**  
    `args(uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_CHANGING_LOCATION(2041)**  
    `args(uint32 do_id, uint32 new_parent_id, uint32 new_zone_id,
                        uint32 old_parent_id, uint32 old_zone_id)`  
> A set location message moves receiving objects to a new location.
>
> A changing location message is sent to notify others of the change:
>  - If an object had an old parent, it is broadcast to the object's location and the old parent.
>  - If an object a new parent, it is broadcast to the new parent.
>  - If an object has an AI, it is broadcast to the AI.
>  - If an object has an owner, it is broadcast to the owner.
> The notification's "sender" is set to the original message's sender.
>
> Then the objects will broadcast one of the following enter location messages to the new location.


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


**STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED(2066)**
    `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST>)`
**STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER(2067)**
    `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST>, <OTHER_BCAST>)`
> Identical to OBJECT_ENTER_LOCATION except for the context at the beginning,
> this message exists to differentiate object entry initiated by a GET_ZONES_OBJECT-type
> query from normal object entry.


**STATESERVER_OBJECT_GET_LOCATION(2044)** `args(uint32 context)`  
**STATESERVER_OBJECT_GET_LOCATION_RESP(2045):**  
    `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id)`  
> Get the location from one or more objects.


**STATESERVER_OBJECT_LOCATION_ACK(2046)** `args(uint32 parent_id, uint32 zone_id)`  
> Sent by the parent to the child to indicate when it has received the
> CHANGING_LOCATION notice.


**STATESERVER_OBJECT_SET_AI(2050)** `args(uint64 ai_channel)`  
**STATESERVER_OBJECT_CHANGING_AI(2051)**  
    `args(uint32 doid_id, uint64 new_ai_channel, uint64 old_ai_channel)`  
> A set AI message moves receiving objects to a new AI.
>
> A changing location message is sent to notify others of the change:
>  - If an object had an old ai, notify the object's old ai.
>  - If an object has children, notify the children using the parent channel (1 << 32|parent_id).
> The notification's "sender" is set to the original message's sender.
>
> Children whose AI channels have not been explicitly set will process the message
> as if they had received a set AI message with ai_channel equal to the new AI;
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
> channel has not been explicitly set, it will act as if it had received a set
> AI message; sending out its own changing and entering AI messages.


**STATESERVER_OBJECT_SET_OWNER(2060)** `args(uint64 owner_channel)`
**STATESERVER_OBJECT_CHANGING_OWNER(2061)**  
    `args(uint32 do_id, uint64 new_owner_channel, uint64 old_owner_channel)`  
> A set owner message moves receiving object to a new owner.
>
> A changing owner message is sent to notify others of the change:
>  - If an object had an old owner, notify the object's old owner.
> The notification's "sender" is set to the original message's sender.
>
> Then the objects will send one of the following enter owner messages to the
> new owner.


**STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED(2062):**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST_OR_OWNRECV>)`  
**STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER(2063):**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id,
          uint16 dclass_id, <REQUIRED_BCAST_OR_OWNRECV>, <OTHER_BCAST_OR_OWNRECV>)`  
> Used by the object to tell the new owner about the object's entry.
>
> The message format is identical to OBJECT_CREATE except that only fields with
> the broadcast or ownrecv keyword are included in the object serialization.
> Other fields are not sent, because the owner may not be privy to those fields.


#### Section 2.3: Parent Object Methods ####
These messages are sent to a single parent object to interact with its children.

**STATESERVER_OBJECT_GET_ZONE_OBJECTS(2100)**  
    `args(uint32 context, uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_GET_ZONES_OBJECTS(2102)**  
    `args(uint32 context, uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
**STATESERVER_OBJECT_GET_CHILDREN(2104)**  
    `args(uint32 context, uint32 parent_id)`  
> Get all child objects in one or more zones from a single object.
>
> The parent will reply immediately with a GET_{ZONE,ZONES,CHILD}_COUNT_RESP
> message. Each object will reply with a STATESERVER_OBJECT_ENTER_LOCATION message.
>
> _Note: If a shard crashes the number of objects may not be correct, as such
>        a client (for ADD_INTEREST) or AI/Uberdog (in the general case) should
>        stop waiting after a reasonable timeout.  In some cases, it may be
>        acceptable or even preferred to not wait for all responses to come in
>        and just act on objects as they come in._


**STATESERVER_OBJECT_GET_ZONE_COUNT(2110)**  
    `args(uint32 context, uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_GET_ZONE_COUNT_RESP(2111)**  
    `args(uint32 context, uint32 object_count)` // when using uint64 ids, `object_count` is uint64.  
**STATESERVER_OBJECT_GET_ZONES_COUNT(2112)**  
    `args(uint32 context, uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
**STATESERVER_OBJECT_GET_ZONES_COUNT_RESP(2113)**  
    `args(uint32 context, uint32 object_count)` // when using uint64 ids, `object_count` is uint64.  
**STATESERVER_OBJECT_GET_CHILD_COUNT(2114)**  
    `args(uint32 context, uint32 parent_id)`  
**STATESERVER_OBJECT_GET_CHILD_COUNT_RESP(2115)**  
    `args(uint32 context, uint32 object_count)` // when using uint64 ids, `object_count` is uint64.  
> Get the number of children objects in one or more of a single object's zones.


**STATESERVER_OBJECT_DELETE_ZONE(2120)**  
    `args(uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_DELETE_ZONES(2122)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
**STATESERVER_OBJECT_DELETE_CHILDREN(2124)**  
    `args(uint32 parent_id)`  
> Delete all objects in one or more zones by forwarding this message to the
> parent's children over the parent messages channel (1 << 32|parent_id).
>
> Children behave as if they had received a DELETE_RAM.
> Children who receive a DELETE_{ZONE,ZONES,CHILDREN} should not send a
> CHANGING_LOCATION message back to the parent.

**STATESERVER_GET_ACTIVE_ZONES(2125)**  
    `args(uint32 context)`  
**STATESERVER_GET_ACTIVE_ZONES_RESP(2126)**  
    `args(uint32 context, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> Gets the list of zones that contain objects



#### Section 2.4: Database-State Objects ####
The database-state server is a special subclass of the stateserver. Unlike a
normal state server, the DBSS does not listen on a control channel and so cannot
receive GENERATE messages. Objects must first be generated by the
DBSERVER_OBJECT_CREATE message in a database before they can be interacted with
the database stateserver.

The database stateserver otherwise provides equivelant StateServer-like behavior
to stored objects in the database.

**DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS(2200)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id)`  
**DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER(2201)**  
    `args(uint32 do_id, uint32 parent_id, uint32 zone_id, uint16 dclass_id,
          uint16 field_count, uint<OTHER_UPDATES>)`  
> Load an object into ram from disk with the given parent and zone.
> The loaded object will use fields from the database if they exist, and will use
> the defaults provided by the dc file for required values if the value is not
> available in the database.
>
> _Note: If no default was provided, the dcparser sets that default to the null-
>        value for that data-type; typically 0._
>
> Once in ram, the DBSS will cache all ram and required fields.
> When the fields are updated on the DBSS, they will be written back to the database.
> An activated object will also listen for broadcasted Database updates and change
> any cached values.
>
> These message will broadcast ENTER_LOCATION and CHANGING_ZONE messages as if the
> object had just been generated with STATESERVER_CREATE_OBJECT.
>
> In the case of `_OTHER`, fields can be manually provided that override values
> from the databases and/or defaults from the DC file.  
> If the wrong dclass_id is sent, the DBSS will ignore the message.


**DBSS_OBJECT_GET_ACTIVATED(2207)** `args(uint32 context, uint32 do_id)`  
**DBSS_OBJECT_GET_ACTIVATED_RESP(2208):**  
    `args(uint32 context, uint32 do_id, bool is_activated)`  
> Tests whether a particular object id has been activated on the DBSS.  
> For a doid which doesn't exist, the message always returns false.


**DBSS_OBJECT_DELETE_FIELD_DISK(2230)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**DBSS_OBJECT_DELETE_FIELDS_DISK(2231)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> Delete one or more db field(s) of a single object.
>
> The message is also used to inform others of the delete by the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The delete-notifications's sender is equal to the original message's sender.
>
> In DELETE_FIELDS, there are multiple field deletes in one message, which will
> be processed as an atomic operation.


**DBSS_OBJECT_DELETE_DISK(2232)**  
    `args(uint32 do_id)`  
> Delete the object from the Database. The object still exists in ram, if it
> was previously activated.
>
> The message is also used to inform others of the delete.  It is broadcast to
> the objects location, managing AI, and owner if they exists.
>
> _Note: Currently this message does not propogate to children. This behavior
>        should be reviewed to determine whether it is expected or not._
