OpenOTP Internal Protocol Documentation
---------------------------------------
**Authors**  
Sam "CFSworks" Edwards (08-30-2013)  
Kevin "Kestred" Stenerson (09-04-2013)


### Section 0: Abstract ###

The purpose of this document is to be a complete reference describing how to
communicate within the OpenOTP cluster. At the heart of every OpenOTP cluster
is a backbone consisting of one or more "Message Directors" which route
messages between the various nodes in the cluster. This document describes the
exact protocol used by the MDs to communicate, as well as the various types of
messages that one can expect to see in a working OpenOTP environment.



### Section 1: Anatomy of a message ###

OpenOTP Message Directors are linked via TCP streams. The linking forms a
tree; that is, no routing loops may exist in a Message Director network. If a
routing loop exists, the traffic will be amplified into a broadcast storm.

When any TCP connection is established, there is an initiator (or connector)
and a receiver (or listener). This distinction is important in the OpenOTP
Message Director hierarchy: One Message Director should be configured as a
root node, which makes no connections to any other nodes. Instead, the root
node listens for connections from other Message Directors, which may either
host other components of the OpenOTP cluster, or they may themselves listen
for connections from other MDs.

Once the TCP connection is open, there is no handshaking process required. The
connecting MD may simply begin sending messages. The format of a typical
message is described below:

    uint16 length;
    uint8 recipient_count;
    uint64 recipients[recipient_count];
    uint64 sender;
    uint16 msgtype;
    ...the message may have a payload following msgtype...

All multi-byte types are to be sent in little-endian order. The "length" tag
represents the length of the message, in bytes, NOT counting itself.
Therefore, consider this example message:

    1a 00                   // Length: count of the following bytes
    01                      // One recipient channel
    d2 04 00 00 00 00 00 00 // Recipient channel ID = 1234
    e1 10 00 00 00 00 00 00 // Sender channel ID = 4321
    39 05                   // Message type code 1337.
    05 00 48 45 4c 4c 4f    // Payload for the message; in this example, the string
                               "HELLO" -- strings are prefixed with a length field of
                               their own, hence the 05 00


#### Section 1.1 DistributedObject Serialization ####
Some messages contained serialized data in their payload.  
Serialization as used in messages means sending individual
variables as raw little-endian byte data, with one value
immediately following the previous value. String type values
are always prefixed with a uint16 length, which is followed
by the raw string data.

The standard format for sending the full encoding of a
DistributedObject is to send, in low to high order of field_id,
the serialized data of all of the required fields of the object.
The required is followed by a uint16 which specifices how many
optional fields exist. If any optional fields exist, this is
followed by <uint16 field_id, [DATA]> pairs for each optional
field. The fields may be in any order.

Example DC Class:

    dclass DistributedAvatar
        name(string n) required db;  // ID = 0 | Value = "Throgdar"
        x(uint64 x) broadcast ram;   // ID = 1 | Value = 5
        y(uint64 y) broadcast ram;   // ID = 2 | No Value
        z(uint64 z) broadcast ram;   // ID = 3 | Value = 0

Serialized form:

    08 00 // Length of "Throgdar"
    54 68 72 6f 67 64 61 72 // Character data
    02 00 // Count of optional fields
    01 00 // field_id for 'x' (ID = 1)
    05 00 00 00 00 00 00 00 // Value = 5
    03 00 // field_id for 'z' (ID = 3)
    00 00 00 00 00 00 00 00 // Value = 0



### Section 2: Control messages ###

As Message Directors operate on a publish-subscribe model, a message will only
be sent downlink (i.e. from a listener to a connector) if the connector
specifically requested to be informed of messages on one of the message's
channels. Uplink messages, however, are sent unsolicited.

To request messages on a channel, the upstream Message Director must receive a
control message from a downstream MD requesting to be added to a channel.

Control messages are distinguished by two things:

1. Control messages must be sent to only channel 4001, and no other channels.
2. Control messages OMIT the sender field; this is because the sender is
   assumed (known) to be the MD on the other end of the link.

The following control messages exist, with their respective formats:

**CONTROL_SET_CON_NAME(2004)** `args(string)`  
**CONTROL_SET_CON_URL(2005)** `args(string)`  
> As every OpenOTP daemon may include a webserver with debug information, it is
often helpful to understand the purpose of incoming MD connections. A
downstream MD may be configured with a specific name, and it may wish to
inform the upstream MD what its name and webserver URL are. These control
messages allow the downstream MD to communicate this information.


**CONTROL_ADD_POST_REMOVE(2010)** `args(string)`  
**CONTROL_CLEAR_POST_REMOVE(2011)** `args()`  
> Often, Message Directors may be unexpectedly disconnected from one another, or
a Message Director may crash while under normal operation without the chance
to clean up. These control messages allow a downstream MD to schedule messages
on the upstream MD to be sent in the event of an unexpected disconnect.

> The argument to CONTROL_ADD_POST_REMOVE is a string; the string contains a
message, minus the length tag (since the string already includes a length tag
of its own, this would be redundant information).
CONTROL_CLEAR_POST_REMOVE is used to reset all of the on-disconnect messages.
This may be used prior to a MD's clean shutdown, if it doesn't wish the
unexpected-disconnect messages to be processed.


**CONTROL_ADD_CHANNEL(2001)** `args(uint64)`  
**CONTROL_REMOVE_CHANNEL(2002)** `args(uint64)`  
> These messages allow a downstream Message Director to (un)subscribe a channel.
The argument is the channel to be added or removed from the subscriptions.


**CONTROL_ADD_RANGE(2008)** `args(uint64, uint64)`  
**CONTROL_REMOVE_RANGE(2009)** `args(uint64, uint64)`  
> These messages add/remove an entire range of channels at once. The first
argument(s) should be the lower channel to add. The second argument(s) is the
upper channel of the range. The ranges are inclusive.



### Section 3: State Server messages ###

This section documents the messages involved in interacting with a State
Server. A State Server is a component with a single, preconfigured channel.
This channel is used to request the State Server to instantiate new objects.
When an object is instantiated, the object behaves as if it were its own
Message Director participant, and subscribes to its own channel (equal to the
object's ID) to receive object-specific updates. Therefore, the functions of
the State Server's control channel are very narrow compared to the wide range
of control afforded by communicating to an instantiated object directly.


#### Section 3.1: State Server control messages ####

These messages are to be sent directly to the State Server's configured
control channel:

**STATESERVER_OBJECT_GENERATE_WITH_REQUIRED(2001)**  
    `args(uint32 parent_id, uint32 zone_id, uint16 dclass_id, uint32 do_id, ...)`  
**STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER(2003)**  
    `args(uint32 parent_id, uint32 zone_id, uint16 dclass_id, uint32 do_id, ...)`  
> Create an object on the State Server, specifying its initial location
as (parent_id, zone_id) and its object type and ID. The ... is an in-order
serialization of all object fields with the "required" keyword.
In the case of *_OTHER, the required fields are followed by a uint16, which
specifies how many additional fields the message contains. Each field is then
a uint16 (representing the field number) followed by the serialized field.


**STATESERVER_SHARD_RESET(2061)** `args(uint64 ai_channel)`  
> Used by an AI Server to inform the State Server that it is going down. The
State Server will then delete all objects matching the ai_channel.
The AI will typically hang this on its connected MD using ADD_POST_REMOVE, so
that the message goes out automatically if the AI loses connection
unexpectedly.


#### Section 3.2: Object control messages ####

These messages are to be sent to the objects themselves. Objects subscribe to
a channel with their own object ID, and therefore can be reached directly by
using their ID as the channel.

**STATESERVER_OBJECT_UPDATE_FIELD(2004)**  
    `args(uint32 do_id, uint16 field, VALUE)`  
**STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE(2005)**  
    `args(uint32 do_id, uint16 num_fields, [uint16 field, VALUE]*num_fields)`  
> Handle a field update on this object. Note that the object MAY ALSO SEND this
message to inform others of an update. If the field is ownrecv, the message
will get sent to the owning-client's channel. If airecv, the message will get
sent to the owning AI Server's channel. If broadcast, the message will get
sent to the location-channel (i.e. (parent_id<<32)|zone_id) so all clients
with interest on that location may see the update.

> In _MULTIPLE, there are multiple field updates in one message, with the
intention that the updates will be processed atomically.


**STATESERVER_OBJECT_DELETE_RAM(2007)** `args(uint32 do_id)`  
**STATESERVER_OBJECT_DELETE_DISK(2060)** `args(uint32 do_id)`  
> Remove the object from the State Server.
RAM (handled by SS) does not affect a stored copy on the database, if one exists.  
DISK (handled by DBSS) deletes a copy of the field on the database.  
The object will duly broadcast its deletion to any AIs, owners, or zones.


**STATESERVER_OBJECT_SET_ZONE(2008)** `args(uint32 parent_id, uint32 zone_id)`  
> Moves the object to a new location (specified by parent_id, zone_id).
The object will inform any AIs and owners of this change, as well as broadcast
its presence in the new zone and its absence in the old zone.


**STATESERVER_OBJECT_CHANGE_ZONE(2009)**  
    `args(uint32 do_id, uint32 new_parent, uint32 new_zone,
                        uint32 old_parent, uint32 old_zone)`  
**STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER(2066)**  
    `(uint16 dclass_id, uint32 do_id, uint32 parent_id, uint32 zone_id, ...)`  
> These messages are SENT BY THE OBJECT when processing a SET_ZONE.
CHANGE_ZONE tells everything that can see the object where the object is going.

> ENTER_ZONE tells the new zone about the object's entry. The message is ordered
slightly differently, but is otherwise identical to the behavior of
STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER.

**STATESERVER_OBJECT_QUERY_ZONE_ALL(2021)**__
    `args(uint32 parent_id, uint16 num_zones, [uint32 zone]*num_zones)`
> Sent to the parent; queries zone(s) for all objects within. Each object will
answer with a STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED(_OTHER). After all
objects have answered, the parent will send:

**STATESERVER_OBJECT_QUERY_ZONE_ALL_DONE(2046)**__
    `args(uint32 parent_id, uint16 num_zones, [uint32 zone]*num_zones)`
> This is an echo of the above message. It is sent back to the enquierer after
all objects have announced their existence.

**STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED(2065)**  
    `args(uint16 dclass_id, uint32 do_id, uint32 parent_id, uint32 zone_id, ...)`  
> Analogous to above, but includes REQUIRED fields only.


**STATESERVER_OBJECT_LOCATE(2022)** `args(uint32 context)`  
**STATESERVER_OBJECT_LOCATE_RESP(2023):**  
    `args(uint32 context, uint32 do_id, uint32 parent_id, uint32 zone_id)`  
> This message may be used to ask an object for its location, returning the response.


**STATESERVER_OBJECT_QUERY_FIELD(2024)**  
    `args(uint32 do_id, uint16 field_id, uint32 context)`  
**STATESERVER_OBJECT_QUERY_FIELD_RESP(2062)**  
    `args(uint32 do_id, uint16 field_id, uint32 context, uint8 success, [VALUE])`  
> This message may be used to ask an object for the value of a field. Returning the
response with a success = 1 if the field is present, or 0(failure) if the field is
nonpresent. Value is not present on failure.


**STATESERVER_OBJECT_SET_AI_CHANNEL(2045)**  
    `args(uint32 do_id, uint64 ai_channel)`  
**STATESERVER_OBJECT_ENTER_AI_RECV(2067)**  
    `args(uint32 parent_id, uint32 zone_id, uint16 class_id, uint32 do_id, ...)`  
**STATESERVER_OBJECT_LEAVING_AI_INTEREST(2033)** `args(uint32 do_id)`  
> Sets the channel for the managing AI. All airecv updates are automatically
forwarded to this channel.  
Note: The managing AI channel can also be set implicitly. If it isn't set
explicitly, it defaults to the AI channel (implicit or explicit) of the
parent.

> ENTER_AI_RECV tells the new AI Server of the object's arrival.
The ... is as in REQUIRED_OTHER.  
LEAVING_AI_INTEREST is sent to the old AI Server to notify it of
the object's departure or deletion.


**STATESERVER_OBJECT_SET_OWNER_RECV(2070)** `args(uint64 owner_channel)`
**STATESERVER_OBJECT_CHANGE_OWNER_RECV(2069)**  
    `args(uint32 do_id, uint64 new_owner_channel, uint64 old_owner_channel)`  
**STATESERVER_OBJECT_ENTER_OWNER_RECV(2068):**  
    `args(uint32 parent_id, uint32 zone_id, uint16 dclass_id, uint32 do_id, ...)`  
> SET_OWNER sets the channel of the object owner. This is the channel of the Client Agent
connection object where ownrecv messages will be forwarded. Similar to changing zone,
this will generate some traffic:  
CHANGE_OWNER will be sent to the old owner.  
ENTER_OWNER tells the new owner of the object's arrival. The ... is as in REQUIRED_OTHER.


**STATESERVER_OBJECT_QUERY_FIELDS(2080)**  
    `args(uint32 do_id, uint32 context, uint16 field_ids[...])`  
> Ask the object for all of its fields.  
Note: there is no length tag on field_ids; the end of the message signifies the
end of the field ID list.


**STATESERVER_OBJECT_QUERY_FIELDS_RESP(2081)**  
    `args(uint32 do_id, uint32 context, uint8 success, [uint16 field_id, VALUE])`  
> The reply sent back to the requester. Again, there is no length tag.

**STATESERVER_OBJECT_QUERY_ALL(2020)** `args(uint32 context)`  
**STATESERVER_OBJECT_QUERY_ALL_RESP(2030)**  
   `args(uint32 context, uint32 parent_id, uint32 zone_id,
         uint16 dclass_id, uint32 do_id, ...)`
> This message queries all information from the object and returns the response.
The ... is like that of REQUIRED_OTHER, above.


**STATESERVER_OBJECT_QUERY_MANAGING_AI(2083)** `args()`  
> _Internally Used_ The object has just changed parents. It wants to know if its
new parent has a different AI Server channel. This message is sent to the new
parent to request that it resend its AI Server channel.


**STATESERVER_OBJECT_NOTIFY_MANAGING_AI(2047)**  
    `args(uint32 parent_id, uint64 ai_channel)`  
> _Internally Used_ Broadcast by a parent_id to all of its children (the channel
for this is given by (4030<<32)|parent_id) when its airecv channel changes.
Also sent on demand in response to STATESERVER_OBJECT_QUERY_MANAGING_AI.



### Section 4: Database Server messages ###

This section documents the messages involved in interacting with a Database
Server. A Database Server is a component with a single, preconfigured channel.
This channel is used to request the Database Server to:
 - Create new objects stored in the database.
 - Update object fields stored in database.
 - Run queries on objects stored in the database.

When a stored object is created directly, the object behaves as if it were its own
Message Director participant, and subscribes to its own channel (equal to the
object's ID) to receive object-specific updates.

The following is a list of database control messages:

**Argument Notes**

    bool success/found      // uint8 value where FAILURE or NOT_FOUND = 0x0 (typically SUCCES or FOUND = 0x1 or "TRUE")

    uint16 dclass_id        // DistributedClass of objects to compare (think MySQL table or mongodb file)

    FIELD_DATA ->           // FIELD_DATA implies the following structure
        (uint16 field_count,    // Number of following fields
            [uint16 field,          // The field of the DistributeClass
             VALUE                  // The serialized value of that field
            ]*field_count)

    COMPARISON_QUERY  ->    // COMPARISON_QUERY implies the following structure
        (uint16 compare_count,  // Number of following comparisons (think SQL WHERE field = value)
            [uint8 compare_op,      // EQUALS(0), NOT_EQUALS(1)
             uint16 field,          // The field of the DistributeClass to compare
             VALUE                  // The serialized value of that field
            ]*compare_count)


**DBSERVER_CREATE_STORED_OBJECT(1003)**  
    `args(uint32 context, uint16 dclass_id, FIELD_DATA)`  
**DBSERVER_CREATE_STORED_OBJECT_RESP(1004)**  
    `args(uint32 context, uint32 do_id)`  
> This message creates a new object in the database with the given fields set to
the given values. For required fields that are not given, the default values
are used.  Objects with required fields that do not have default values cannot
be stored in the database.  
The return is the do_id of the object. The BAD_DO_ID (0x0) is a failure.  
When the object is queried, this object is automatically fetched and managed by
the associated DB-SS.  


**DBSERVER_DELETE_STORED_OBJECT(1008)**  
    `args(uint32 verify_code, uint32 do_id)`  
> This message removes an object from the server with a given do_id.  
The verify_code is 0x44696521 (Ascii "Die!") and is required.  


**DBSERVER_DELETE_QUERY(1010)**  
    `args(uint32 verify_code, uint16 dclass_id, COMPARISON_QUERY)`  
> This message removes all objects from the server of the given DistributedClass,
that satisify the given comparisons.  
The verify_code is 0x4b696c6c (Ascii "Kill") and is required.  


**DBSERVER_SELECT_STORED_OBJECT(1012)**  
    `args(uint32 context, uint32 do_id, uint16 field_count, [uint16 field]*field_count`  
**DBSERVER_SELECT_STORED_OBJECT_RESP(1013)**  
    `args(uint32 context, bool found, [VALUE]*field_count)`  
> This message selects a number of fields from an object in the database.  
It returns the select fields in serialized form, in the order requested -- if found.


**DBSERVER_SELECT_STORED_OBJECT_ALL(1020)**  
    `args(uint32 context, uint32 do_id)`  
**DBSERVER_SELECT_STORED_OBJECT_ALL_RESP(1021)**  
    `args(uint32 context, bool found, [uint16 dclass_id, FIELD_DATA])`  
> This message queries all of the database fields from the object and returns the response -- if found.


**DBSERVER_SELECT_QUERY(1016)**  
    `args(uint32 context, uint32 dclass_id, COMPARISON_QUERY)`  
**DBSERVER_SELECT_QUERY_RESP(1017)**  
    `args(uint32 context, uint32 items, [uint32 do_id]*items)`  
> This message selects from the database all items of the given
DistributedClass that satisfy the given comparisons.
The return is a list of do_ids corresponding to the selected elements.


**DBSERVER_UPDATE_STORED_OBJECT(1014)**  
    `args(uint32 context, uint32 do_id, FIELD_DATA)`  
> This message replaces the current values of the given object,
with the new values given in FIELD_DATA.  
This command updates a value in the database, ignoring its initial value.
If using a SELECT, followed by an UPDATE, see UPDATE_IF_EQUALS instead.


**DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS(1024)**  
    `args(uint32 context, uint32 do_id, uint16 field_count,
          [uint16 field, VALUE old, VALUE new]*field_count)`
**DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP(1025)**  
    `args(uint32 context, bool success, [VALUE]*field_count)`
> This message replaces the current values of the given object with new values,
only if the old values match the current state of the database.  
This method of updating the database is used to prevent race conditions,
particularily when the new values are derived or dependent on the old values.

> If any of the given _old_ values don't match then the entire transaction fails.
If unsuccessful, the current values of all given fields will be returned in order
in serialized form, after 'success'.


**DBSERVER_UPDATE_QUERY(1018)**  
    `args(uint32 dclass_id, COMPARISON_QUERY, FIELD_DATA)`  
> This message updates all objects of type dclass_id which satisfy the
COMPARISON_QUERY with the fields in FIELD_DATA.

### Section 5: Event Logger ###

The Event Logger's messages are unique in that they do not pass through the
normal Message Director infrastructure. Instead, the Event Logger sits on a UDP
socket waiting for incoming UDP packets. Each packet contains one log message.

The formatting is incredibly simple:

    string sender_name
    string event_type
    [string param1
     [...]]

Note that the implementation itself does not *require* the sender name, event type,
and event parameters in that order. It simply writes each string to its
CSV-formatted log file. However, by convention, the sender name is to be sent
first, followed by the event type, and then all interesting details on that event.

### Section 6: Client Agent ###

The Client Agent is responsible for allowing clients into the network. Clients
**do not** connect directly to the Message Director: this would be a serious
security risk, as clients could then send any message they want in order to
disrupt the running game. Instead, the clients connect to the Client Agent, which
acts as a gateway into the running network.

The client protocol is not the same as the internal inter-MD protocol documented
here. For information on how clients communicate with the server, see the
`client_protocol.md` file in this directory.

#### Section 6.1: Client Agent responsibilities ####

The Client Agent is responsible for all security in the game environment.
Its responsibilities include ensuring clients only send clsend/ownsend messages,
ensuring clients do not send corrupt field updates, verifying client version
upon connection, and so on.

Aside from security, the Client Agent is responsible for retrieving information
from the running cluster on the client's behalf. If a client sends a message
to open interest in a particular zone, the Client Agent will query all objects
residing in that zone.

#### Section 6.2: Client Agent state machine ####

Every connected client resides in one of three states:

- **NEW(0)**: The client has just connected. The CA has no idea what version the
client is running, or even if the client is a game client. In this state, the
client **must** send a CLIENT_HELLO as its first message. Any other data will
result in a disconnect.
- **ANONYMOUS(1)**: The client has just sent a HELLO, but has not provided suitable
authentication yet. In this state, the client may communicate with predesignated
UberDOGs to provide authentication credentials and/or make anonymous game-state
queries. Upon providing suitable authentication, the UberDOG(s) will instruct the
CA, using an internal message, to move the client to the next state.
- **ESTABLISHED(2)**: The client is fully connected and may behave normally. It is
no longer restricted to the "anonymous" subset of UberDOGs.

#### Section 6.3: Client Agent session messages ####

Unlike the State Server, the Client Agent does not have a control channel of its
own. Therefore, all of these messages are meant to be sent to an active client
session channel.


**CLIENTAGENT_OPEN_CHANNEL(3104)**
    `args(uint64 channel)`
> Instruct the client session to open a channel on the MD. Messages sent to this
new channel will be processed by the CA.

**CLIENTAGENT_CLOSE_CHANNEL(3105)**
    `args(uint64 channel)`
> This message is the antithesis of the message above. The channel is immediately
closed, even if the channel was automatically opened.

**CLIENTAGENT_ADD_INTEREST(3106)**
    `args(uint16 interest_id, uint32 parent_id, [uint32 zone_id, uint32 zone_id, ...])`
> This message instructs the CA to open an interest, as if the client had
requested the interest itself.

**CLIENTAGENT_REMOVE_INTEREST(3107)**
    `args(uint16 interest_id)`
> The antithesis of the message above: cause an open interest to be closed. This
is even valid for client-opened interests, if the interest_id matches a client-requested
interest.

**CLIENTAGENT_ADD_POST_REMOVE(3108)**
    `args(string msg)`
> Similar to CONTROL_ADD_POST_REMOVE, this hangs a "post-remove" message on the
client. If the client is ever disconnected, the post-remove messages will be sent
out automatically.

**CLIENTAGENT_CLEAR_POST_REMOVE(3109)**
    `args()`
> Undoes all CLIENTAGENT_ADD_POST_REMOVE messages.

**CLIENTAGENT_DISCONNECT(3101)**
    `args(uint16 code, string reason)`
> Drops the client with the specified code and reason. The code and reason carry
the same meaning as CLIENT_GO_GET_LOST.

**CLIENTAGENT_DROP(3102)**
    `args()`
> Similar to above, but causes the CA to silently close the client connection,
providing no explanation whatsoever to the client.

**CLIENTAGENT_SEND_DATAGRAM(3100)**
    `args(string datagram)`
> Send a raw datagram down the pipe to the client. This is useful for sending
game-specific messages to the client, debugging, etc.

**CLIENTAGENT_SET_SENDER_ID(3103)**
    `args(uint64 channel)`
> Changes the sender used to represent this client. This is useful if game
components need to identify the avatar/account a given message came from: by
changing the sender channel to include this information, the server can easily
determine the account ID of a client that sends a field update. Note that this
also results in the CA opening the new channel, if it isn't open already.

**CLIENTAGENT_SET_STATE(3110)**
    `args(uint16 state)`
> Move the CA's state machine to a given state. This is mainly used when a client
logs in or logs out, to flip the client between the ANONYMOUS and ESTABLISHED
states respectively.

**CLIENTAGENT_ADD_SESSION_OBJECT(3112)**
    `args(uint32 do_id)`
> Declares the specified object to be a "session object" -- an avatar, for example --
that is automatically cleaned up when the client disconnects. In addition, session
objects are presumed to be required for the proper function of a client. Therefore,
if a session object is ever deleted by another process, the client is automatically
dropped.

**CLIENTAGENT_REMOVE_SESSION_OBJECT(3113)**
    `args(uint32 do_id)`
> Antithesis of above message. The declared object is no longer tied to the client's
session, and will therefore not be deleted if the client drops (nor will the client
be dropped if this object is deleted).

**CLIENTAGENT_DECLARE_OBJECT(3114)**
    `args(uint32 do_id, uint16 dclass_id)`
> Because Client Agents verify the integrity of field updates, they must know the
dclass of a given object to ensure that the incoming field update is for a field that
the dclass/object actually has. Therefore, clients are normally unable to send messages
to objects unless they are either configured as an UberDOG or are currently visible
to the client. This message explicitly tells the CA that a given object exists, of
a given type, and allows the client to send field updates to that object even if
the client cannot currently see that object.

**CLIENTAGENT_UNDECLARE_OBJECT(3115)**
    `args(uint32 do_id)`
> Antithesis of above message: the object is no longer explicitly declared, and
the client can no longer send updates on this object without seeing it.

**CLIENTAGENT_SET_FIELDS_SENDABLE(3111)**
    `args(uint32 do_id, [uint16 field_id, uint16 field_id, ...])`
> Override the security on certain fields for a given object. The specified fields
are made sendable by the client regardless of ownsend/clsend. To undo the security
override, send this message again without any field IDs, to clear the list of
security-overridden fields for that object.
