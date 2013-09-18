OpenOTP Client Protocol Documentation
---------------------------------------
**Author**
Sam "CFSworks" Edwards (09-17-2013)

### Section 0: Abstract ###

This file documents the protocol used by clients to communicate with an OpenOTP
cluster. This should not be confused with the *internal protocol*, which is used
by OpenOTP servers to communicate amongst themselves. Instead, the client
protocol is used by game clients (running on the players' computers) to communicate
with the Client Agent -- the gateway into the cluster. Because clients connect
to the Client Agent rather than directly into the Message Director, I will use
the terms "gameserver" and "Client Agent" interchangeably.

### Section 1: Basic messaging format ###

Unlike the internal protocol, the client protocol has a very simple messaging format.

    uint16 length;  // Length of the entire message, excluding the length tag
    uint16 msgtype; // 16-bit identifier indicating what type of message is being sent
    ...             // message-specific data

Like the internal protocol, all multibyte integers are sent little-endian.

### Section 2: Typical client session ###

The client communicates with the server over a standard TCP connection. When the
client first connects, it is expected to send a `CLIENT_HELLO` message. This
message includes the hash of the DC file(s) and a (game-specific) version string.
The Client Agent will disconnect the client if either value differs from the
Client Agent's value. This is to ensure that out-of-date clients do not accidentally
connect to the gameserver.

After the `CLIENT_HELLO` is accepted, the client is in an "unauthenticated" state.
In this state, the client can send messages to certain, pre-approved objects, but
cannot open interest or do anything else. The objects are not announced to the
client: the client is expected to know their IDs in advance.

The purpose of this mechanism is to allow the client to provide suitable authentication
to the gameserver cluster. The authentication mechanism is entirely up to the
game developer: OpenOTP does not mandate any sort of authentication method.

Once the client proves its identity to the server (through the aforementioned
pre-approved objects), the server will unrestrict the client, allowing it full
access to the normal gameserver logic. The client may then open interests to
discover already-existing objects in client-specified locations. The client may
also send field updates on any object, provided the field is tagged clsend.

The client is also permitted to "own" objects. These are objects where the server
grants the client additional privileges on those objects. The client may send
fields marked as "ownsend", receive field updates for fields marked "ownrecv",
and change the location of the owned objects at will.

When the client wishes to disconnect, it may simply close the TCP session.

### Section 3: Client messages ###

These are the messages that the client and Client Agent may send between each other
in order to accomplish various normal game tasks.

**CLIENT_HELLO(???)**
    `args(uint32 dc_hash, string version)`
> This is the first message a client may send. The dc_hash is a 32-bit hash value
calculated from all fields/classes listed in the client's DC file. The version is
a game-specific string that developers should change whenever they release a new
client build. Both values are compared to the Client Agent's DC file hash and
configured version string to ensure that the client is fully up-to-date. If the
client is not up-to-date, it will be disconnected with a `CLIENT_GO_GET_LOST`.
If the client is up-to-date, the gameserver will send a `CLIENT_HELLO_RESP` to
inform the client that it may proceed with its normal logic flow.

**CLIENT_HELLO_RESP(???)**
    `args()`
> This is sent by the Client Agent to the client when the client's `CLIENT_HELLO`
is accepted.

**CLIENT_GO_GET_LOST(4)**
    `args(uint16 error_code, string reason)`
> This is sent by the Client Agent to the client when the client is being
disconnected. The error_code and reason arguments provide some explanation as
to why the client is being dropped from the game.

**CLIENT_OBJECT_UPDATE_FIELD(24)**
    `args(uint32 do_id, uint16 field_id, VALUE)`
> This is sent either by the Client Agent or the client to issue a field update
on a given object. The format of this message is exactly the same as
`STATESERVER_OBJECT_UPDATE_FIELD` in the internal protocol.

**CLIENT_OBJECT_DISABLE(25)**
    `args(uint32 do_id)`
> This is sent by the Client Agent to let the client know that an object is
leaving the client's visibility, either due to deletion, zone change, or dropped
interest.

**CLIENT_CREATE_OBJECT_REQUIRED(34)**

**CLIENT_CREATE_OBJECT_REQUIRED_OTHER(35)**

**CLIENT_CREATE_OBJECT_REQUIRED_OTHER_OWNER(36)**

**CLIENT_HEARTBEAT(52)**
    `args()`
> The client should send this message on a regular interval. If the Client Agent
does not receive a `CLIENT_HEARTBEAT` for a certain (configurable) amount of time,
it will assume that the client has crashed and disconnect the client.

**CLIENT_ADD_INTEREST(97)**

**CLIENT_REMOVE_INTEREST(98)**

**CLIENT_OBJECT_LOCATION(102)**
