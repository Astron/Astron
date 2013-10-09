Event Logger Behavior
---------------------
**Author**
Sam "CFSworks" Edwards (09-17-2013)


### Section 0: Summary ###

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
