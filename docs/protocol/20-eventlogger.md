Event Logger Behavior
---------------------
**Author**
Sam "CFSworks" Edwards (09-17-2013)


### Section 0: Summary ###

The Event Logger's messages are unique in that they do not pass through the
normal Message Director infrastructure. Instead, the Event Logger sits on a UDP
socket waiting for incoming UDP packets. Each packet contains one log message.

The formatting uses MessagePack, and is incredibly simple. Event UDP packets are
to contain one MessagePack-formatted map, arranged as:

    {"type": "type_of_event",
     "sender": "identity_of_sender",
     ...}

Note that the implementation itself does not *require* the event type, sender name,
and event parameters in that order. It simply writes whatever is received to its
JSON-formatted log file. However, by convention, the event type is to be sent
first, followed by the sender name, and then all interesting details on that event.

Also note: While the TCP-based protocol is little-endian, MessagePack is
big-endian. If you are using a MessagePack library, this will be handled for you.
However, you should keep this fact in mind if you are writing your own MessagePack
implementation or inspecting raw packets.
