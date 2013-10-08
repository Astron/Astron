Client Agent Behavior
---------------------
**Author**
Sam "CFSworks" Edwards (09-17-2013)


### Section 0: Abstract ###

The Client Agent is responsible for allowing clients into the network. Clients
**do not** connect directly to the Message Director: this would be a serious
security risk, as clients could then send any message they want in order to
disrupt the running game. Instead, the clients connect to the Client Agent, which
acts as a gateway into the running network.

The client protocol is not the same as the internal inter-MD protocol documented
here. For information on how clients communicate with the server, see the
`01-client.md` file in this directory.


### Section 1: Responsibilities ###

The Client Agent is responsible for all security in the game environment.
Its responsibilities include ensuring clients only send clsend/ownsend messages,
ensuring clients do not send corrupt field updates, verifying client version
upon connection, and so on.

Aside from security, the Client Agent is responsible for retrieving information
from the running cluster on the client's behalf. If a client sends a message
to open interest in a particular zone, the Client Agent will query all objects
residing in that zone.


### Section 2: CA State Machine ###

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


### Section 3: Session Messages ###

Unlike the State Server, the Client Agent does not have a control channel of its
own. Therefore, all of these messages are meant to be sent to an active client
session channel.


**CLIENTAGENT_OPEN_CHANNEL(3104)** `args(uint64 channel)`  
> Instruct the client session to open a channel on the MD. Messages sent to this
new channel will be processed by the CA.

**CLIENTAGENT_CLOSE_CHANNEL(3105)** `args(uint64 channel)`  
> This message is the antithesis of the message above. The channel is immediately
closed, even if the channel was automatically opened.

**CLIENTAGENT_ADD_INTEREST(3106)**  
    `args(uint16 interest_id, uint32 parent_id, uint32 zone_id)`  
> This message instructs the CA to open an interest, as if the client had
requested the interest itself.

**CLIENTAGENT_ADD_INTEREST_MULTIPLE(????)**  
    `args(uint16 interest_id, uint32 parent_id, uint16 zone_count, [uint32 zone_id]*zone_count)`  
> This message instructs the CA to open an interest, as if the client had
requested the interest itself.

**CLIENTAGENT_REMOVE_INTEREST(3107)** `args(uint16 interest_id)`  
> The antithesis of the message above: cause an open interest to be closed. This
is even valid for client-opened interests, if the interest_id matches a client-requested
interest.

**CLIENTAGENT_ADD_POST_REMOVE(3108)** `args(blob datagram)`  
> Similar to CONTROL_ADD_POST_REMOVE, this hangs a "post-remove" message on the
client. If the client is ever disconnected, the post-remove messages will be sent
out automatically.

**CLIENTAGENT_CLEAR_POST_REMOVE(3109)** `args()`  
> Undoes all CLIENTAGENT_ADD_POST_REMOVE messages.

**CLIENTAGENT_DISCONNECT(3101)**  
    `args(uint16 disconnect_code, string reason)`  
> Drops the client with the specified code and reason. The code and reason carry
the same meaning as CLIENT_GO_GET_LOST.

**CLIENTAGENT_DROP(3102)** `args()`  
> Similar to above, but causes the CA to silently close the client connection,
providing no explanation whatsoever to the client.

**CLIENTAGENT_SEND_DATAGRAM(3100)** `args(blob datagram)`  
> Send a raw datagram down the pipe to the client. This is useful for sending
game-specific messages to the client, debugging, etc.

**CLIENTAGENT_SET_SENDER_ID(3103)** `args(uint64 channel)`  
> Changes the sender used to represent this client. This is useful if game
components need to identify the avatar/account a given message came from: by
changing the sender channel to include this information, the server can easily
determine the account ID of a client that sends a field update. Note that this
also results in the CA opening the new channel, if it isn't open already.

**CLIENTAGENT_SET_STATE(3110)** `args(uint16 ca_state)`  
> Move the CA's state machine to a given state. This is mainly used when a client
logs in or logs out, to flip the client between the ANONYMOUS and ESTABLISHED
states respectively.

**CLIENTAGENT_ADD_SESSION_OBJECT(3112)** `args(uint32 do_id)`  
> Declares the specified object to be a "session object" -- an avatar, for example --
that is automatically cleaned up when the client disconnects. In addition, session
objects are presumed to be required for the proper function of a client. Therefore,
if a session object is ever deleted by another process, the client is automatically
dropped.

**CLIENTAGENT_REMOVE_SESSION_OBJECT(3113)** `args(uint32 do_id)`  
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

**CLIENTAGENT_UNDECLARE_OBJECT(3115)** `args(uint32 do_id)`  
> Antithesis of above message: the object is no longer explicitly declared, and
the client can no longer send updates on this object without seeing it.

**CLIENTAGENT_SET_FIELDS_SENDABLE(3111)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> Override the security on certain fields for a given object. The specified fields
are made sendable by the client regardless of ownsend/clsend. To undo the security
override, send this message again without any field IDs, to clear the list of
security-overridden fields for that object.
