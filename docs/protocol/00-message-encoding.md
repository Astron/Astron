Description of Messages & Data Encoding
---------------------------------------
**Authors**  
Sam "CFSworks" Edwards (08-30-2013)  
Kevin "Kestred" Stenerson (09-04-2013)


### Section 0: Anatomy of a Message ###

Astron Message Directors are linked via TCP streams. The linking forms a
tree; that is, no routing loops may exist in a Message Director network. If a
routing loop exists, the traffic will be amplified into a broadcast storm.

When any TCP connection is established, there is an initiator (or connector)
and a receiver (or listener). This distinction is important in the Astron
Message Director hierarchy: One Message Director should be configured as a
root node, which makes no connections to any other nodes. Instead, the root
node listens for connections from other Message Directors, which may either
host other components of the Astron cluster, or they may themselves listen
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


#### Section 1: DistributedObject Serialization ####
Some messages contained serialized data in their payload.  
Serialization as used in messages means sending individual
variables as raw little-endian byte data, with one value
immediately following the previous value. String and blob type
values are always prefixed with a uint16 length, which is
followed by the raw string/binary data.

The standard format for sending the full encoding of a
DistributedObject is to send, in low to high order of field_id,
the serialized data of all of the required fields of the object.
The required is followed by a uint16 which specifices how many
optional fields exist. If any optional fields exist, this is
followed by <uint16 field_id, [DATA]> pairs for each optional
field. It is RECOMMENDED that they be in low to high order of
field_id.  Astron will always broadcast fields in that order.

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

### Section 2: Argument Types ###
The following are the types used as message arguments:

 - *bool:* A single byte integer containg either 0 (False) or 1 (True).
 - *blob:* A length-prefixed array of raw-byte data.
 - *string:* A length-prefixed array of character data (no null-terminator).
 - *uintN:* An N-bit little-endian encoded integer value.

### Section 3: Special Argument Shorthand ###
    VALUE                   // VALUE is the serialization of a DistrubutedObject field

    REQUIRED ->             // REQUIRED is an inline of
        <VALUE>*required_count  // All of the fields with the 'required' keyword in
                                // ascending order by field_id.

    OTHER -> <FIELD_DATA>   // OTHER is like field_data but only contains any remaining
                            // optional fields that may be provided by the caller.


### Section 4: Message-type Ranges ###
Each component of the Astron daemon is given its own message-type range:

 - **Client:**              1 to  999
 - **Client Agent:**     1000 to 1999
 - **State Server:**     2000 to 2999
 - **Database Server:**  3000 to 3999
 - _Reserved:_           4000 to 8999
 - **Message Director:** 9000 to 9999

 ### Section 5: Reserved Channels | Object IDs ###

 - **Invalid Channel:**         0
 - **Control Messages:**        1
 - **All Clients:**            10
 - **All State Servers:**      12
 - **All Database Servers:**   13
 - _Reserved Channels:_      <999

 ### Section 6: Reserved Ranges ###

 - **Parent Messages:**     1 << 32
 - _Reserved Ranges:_    <999 << 32
