Controlling the Message Director
-------------------------------------
**Authors**  
Sam "CFSworks" Edwards (08-30-2013)  
Kevin "Kestred" Stenerson (09-04-2013)


### Section 0: Control Messages ###

As Message Directors operate on a publish-subscribe model, a message will only
be sent downlink (i.e. from a listener to a connector) if the connector
specifically requested to be informed of messages on one of the message's
channels. Uplink messages, however, are sent unsolicited.

To request messages on a channel, the upstream Message Director must receive a
control message from a downstream \MD requesting to be added to a channel.

Control messages are distinguished by two things:

1. Control messages must be sent only to channel 1, and no other channels.
2. Control messages OMIT the sender field; this is because the sender is
   assumed (known) to be the MD on the other end of the link.

The following control messages exist, with their respective formats:

**CONTROL_ADD_CHANNEL(9000)** `args(uint64 channel)`  
**CONTROL_REMOVE_CHANNEL(9001)** `args(uint64 channel)`  
> These messages allow a downstream Message Director to (un)subscribe a channel.
> The argument is the channel to be added or removed from the subscriptions.


**CONTROL_ADD_RANGE(9002)**  
`args(uint64 low_channel, uint64 high_channel)`  
**CONTROL_REMOVE_RANGE(9003)**  
`args(uint64 low_channel, uint64 high_channel)`  
> These messages add/remove an entire range of channels at once. The first
> argument(s) should be the lower channel to add. The second argument(s) is the
> upper channel of the range. The ranges are inclusive.


**CONTROL_ADD_POST_REMOVE(9010)** `args(uint64 sender, blob datagram)`  
**CONTROL_CLEAR_POST_REMOVES(9011)** `args(uint64 sender)`  
> Often, Message Directors may be unexpectedly disconnected from one another, or
> a Message Director may crash while under normal operation without the chance
> to clean up. These control messages allow a downstream MD to schedule messages
> on the upstream MD to be sent in the event of an unexpected disconnect.
>
> The sender is the channel (typically representing the participant who sends the message)
> that the post removes should be tied to.  This field is only used to be able to clear a
> bundle of post removes for a particular sender.  Unlike other messages, post removes
> MUST NOT be sent by Roles or AIs with a feigned sender -- the post remove is only sent
> when the participant that sent it disconnects.
>
> The second argument to CONTROL_ADD_POST_REMOVE is a blob; the blob contains a
> message, minus the length tag (since the blob already includes a length tag
> of its own, this would be redundant information).
> CONTROL_CLEAR_POST_REMOVE is used to reset all of the on-disconnect messages.
> This may be used prior to a MD's clean shutdown, if it doesn't wish the
> unexpected-disconnect messages to be processed.


**CONTROL_SET_CON_NAME(9012)** `args(string name)`  
**CONTROL_SET_CON_URL(9013)** `args(string url)`  
> As every Astron daemon may include a webserver with debug information, it is
> often helpful to understand the purpose of incoming MD connections. A
> downstream MD may be configured with a specific name, and it may wish to
> inform the upstream MD what its name and webserver URL are. These control
> messages allow the downstream MD to communicate this information.

**CONTROL_LOG_MESSAGE(9014)** `args(blob message)`
> In production layouts, it may be useful for AIs to log messages
> to the eventlogger infrastructure (preferably a fluentd instance) without
> needing to have redundant configuration on the AI servers, which could come
> out of sync. Using this message, the MD will simply route the message argument
> to the configured eventlogger.
