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


**STATESERVER_DELETE_AI_OBJECTS (2007)** `args(uint64 ai_channel)`  
> Used by an AI Server to inform the State Server that it is going down. The
> State Server will then delete all objects matching the ai_channel.
>
> The AI will typically hang this on its connected MD using ADD_POST_REMOVE, so
> that the message goes out automatically if the AI loses connection unexpectedly.


### Section 2: Distributed Object Control Messages ###

These messages are to be sent to the objects themselves. Objects subscribe to
a channel with their own object ID, and therefore can be reached directly by
using their ID as the channel.

**STATESERVER_OBJECT_SET_FIELD(2015)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**STATESERVER_OBJECT_SET_FIELDS(2016)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> These messages are used to set one or more field(s) on the distributed object.  
> The message is also used to inform others of the change with the following:
> - An airecv field will be sent to the object's AI channel.
> - An ownrecv field will be sent to the owning-client's channel.
> - A broadcast field will be sent to the location-channel (parent_id<<32)|zone_id).
> The change message's sender is equal to the original message's sender.
>
> In SET_FIELDS, there are multiple field updates in one message, which will be
> processed as an atomic operation. Note, in the case of field duplicates, the
> last value in the message is used.
>
> _Notes: The doid argument in this message is used for identification by
>         update-recievers as well as for inactive objects in a dbss._


**STATESERVER_OBJECT_DELETE_RAM(2007)** `args()`  
**STATESERVER_OBJECT_DELETE_DISK(2060)** `args()`  
> Remove the object from the State Server.
RAM (handled by SS) does not affect a stored copy on the database, if one exists.  
DISK (handled by DBSS) deletes a copy of the field on the database.  
The object will duly broadcast its deletion to any AIs, owners, or zones.


**STATESERVER_OBJECT_SET_ZONE(2008)** `args(uint32 parent_id, uint32 zone_id)`  
> Moves the object to a new location (specified by parent_id, zone_id).
The object will inform any AIs and owners of this change, as well as broadcast
its presence in the new zone and its absence in the old zone.


**STATESERVER_OBJECT_CHANGE_ZONE(2009)**  
    `args(uint32 do_id, uint32 new_parent_id, uint32 new_zone_id,
                        uint32 old_parent_id, uint32 old_zone_id)`  
**STATESERVER_OBJECT_ENTER_ZONE_WITH_REQUIRED_OTHER(2066)**  
    `(uint32 do_id, uint32 parent_id, uint32 zone_id, uint16 dclass_id, <REQUIRED>, <OTHER>)`  
> These messages are SENT BY THE OBJECT when processing a SET_ZONE.
CHANGE_ZONE tells everything that can see the object where the object is going.  

> ENTER_ZONE tells the new zone about the object's entry. The message is ordered
slightly differently, but is otherwise identical to the behavior of
STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER.


**STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED(2065)**
    `args(uint32 parent_id, uint32 zone_id, uint16 dclass_id, uint32 do_id, ...)`
> Analogous to above, but includes REQUIRED+BROADCAST fields only, no OTHER.

**STATESERVER_OBJECT_QUERY_ZONE_ALL(????)**  
    `args(uint32 parent_id, uint32 zone_id)`  
**STATESERVER_OBJECT_QUERY_ZONES_ALL(2021)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> Sent to the parent; queries zone(s) for all objects within. Each object will
answer with a STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED(_OTHER). After all
objects have answered, the parent will send the corresponding _DONE message.

**STATESERVER_OBJECT_QUERY_ZONE_DONE(????)**
    `args(uint32 parent_id, uint32 zone_id)`
**STATESERVER_OBJECT_QUERY_ZONES_ALL_DONE(2046)**  
    `args(uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> These are echoes of the above messages. They are sent back to the enquierer after
all objects have announced their existence.


**STATESERVER_OBJECT_LOCATE(2022)** `args(uint32 context)`  
**STATESERVER_OBJECT_LOCATE_RESP(2023):**  
    `args(uint32 context, uint32 parent_id, uint32 zone_id)`  
> This message may be used to ask an object for its location, returning the response.


**STATESERVER_OBJECT_QUERY_FIELD(2024)**  
    `args(uint32 context, uint16 field_id)`  
**STATESERVER_OBJECT_QUERY_FIELD_RESP(2062)**  
    `args(uint32 context, uint8 success, [uint16 field_id, <VALUE>])`  
> This message may be used to ask an object for the value of a field.
Returns failure(0) if the field is nonpresent.


**STATESERVER_OBJECT_SET_AI_CHANNEL(2045)**  
    `args(uint64 ai_channel)`  
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


**STATESERVER_OBJECT_QUERY_FIELDS(2080)**  
    `args(uint32 context, uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
**STATESERVER_OBJECT_QUERY_FIELDS_RESP(2081)**  
    `args(uint32 context, uint32 do_id, uint8 success, <FIELD_DATA>)`  
> This message asks for multiple fields to be recieved at the same time.


**STATESERVER_OBJECT_QUERY_ALL(2020)** `args(uint32 context)`  
**STATESERVER_OBJECT_QUERY_ALL_RESP(2030)**  
   `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id,
         uint16 dclass_id, <REQUIRED>, <OTHER>)`
> This message queries all information from the object and returns the response.


**STATESERVER_OBJECT_QUERY_MANAGING_AI(2083)** `args()`  
> _Internally Used_ The object has just changed parents. It wants to know if its
new parent has a different AI Server channel. This message is sent to the new
parent to request that it resend its AI Server channel.


**STATESERVER_OBJECT_NOTIFY_MANAGING_AI(2047)**  
    `args(uint32 parent_id, uint64 ai_channel)`  
> _Internally Used_ Broadcast by a parent_id to all of its children (the channel
for this is given by (4030<<32)|parent_id) when its airecv channel changes.
Also sent on demand in response to STATESERVER_OBJECT_QUERY_MANAGING_AI.


