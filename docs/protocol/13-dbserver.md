Database-Server Behavior
------------------------
**Authors**  
Kevin "Kestred" Stenerson (09-04-2013)


### Section 0: Abstract ###

This section documents the messages involved in interacting with a Database
Server. A Database Server is a component with a single, preconfigured channel.
This channel is used to request the Database Server to:
 - Create new objects stored in the database.
 - Update object fields stored in database.
 - Run queries on objects stored in the database.

When one or more of a database object's values is updated through a set- or delete-
operation, the database will broadcast the received message over the database messages
channel (2 << 32|do_id).  This behavior is default, but can be disabled through the daemon config.


### Section 1: Database Server Messages ###
The following is a list of database control messages:

**DBSERVER_CREATE_OBJECT(3000)**  
    `args(uint32 context, uint16 dclass_id, uint16 field_count,
         [uint16 field_id, <VALUE>]*field_count)`  
**DBSERVER_CREATE_OBJECT_RESP(3001)**  
    `args(uint32 context, uint32 do_id)`  
> This message creates a new object in the database with the given fields set to
> the given values.  If a field was not given, but has a default value, the default
> value is stored.
> 
> If creation fails INVALID_DO_ID (0) is returned.


**DBSERVER_OBJECT_GET_FIELD(3010)**  
    `args(uint32 context, uint32 do_id, uint16 field_id)`  
**DBSERVER_OBJECT_GET_FIELD_RESP(3011)**  
    `args(uint32 context, uint8 success, [uint16 field_id, <VALUE>])`  
> This message gets the value of a single field from an object in the database.
> If the field is not set, the response returns a failure.


**DBSERVER_OBJECT_GET_FIELDS(3012)**  
    `args(uint32 context, uint32 do_id, uint16 field_count,
         [uint16 field_id]*field_count`  
**DBSERVER_OBJECT_GET_FIELDS_RESP(3013)**  
    `args(uint32 context, uint8 success, [uint16 field_count],
         [uint16 field_id, <VALUE>]*field_count)`  
> This message gets the values of multiple fields from an object in the database.
> Database fields with no stored value are not included in the list of returned fields.


**DBSERVER_OBJECT_GET_ALL(3014)**  
    `args(uint32 context, uint32 do_id)`  
**DBSERVER_OBJECT_GET_ALL_RESP(3015)**  
    `args(uint32 context, uint8 success,
         [uint16 dclass_id, uint16 field_count],
         [uint16 field_id, <VALUE>]*field_count)`  
> This message queries all of the data stored in the database about an object.
> Database fields with no stored value are not included in the list of returned fields.


**DBSERVER_OBJECT_SET_FIELD(3020)**  
    `args(uint32 do_id, uint16 field_id, <VALUE>)`  
**DBSERVER_OBJECT_SET_FIELDS(3021)**  
    `args(uint32 do_id, uint16 field_count, [uint16 field_id, <VALUE>]*field_count)`  
> These messages replace an object's current stored values for given fields.
> For updates that are derived or dependent on previous values, consider
> using SET_FIELD(S)_IF_EQUALS message instead.


**DBSERVER_OBJECT_SET_FIELD_IF_EQUALS(3022)**  
    `args(uint32 context, uint32 do_id, uint16 field_id, <VALUE> old, <VALUE> new)`  
**DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP(3023)**  
    `args(uint32 context, uint8 success, [uint16 field_id, <VALUE>])`  
**DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS(3024)**  
    `args(uint32 context, uint32 do_id, uint16 field_count,
         [uint16 field_id, VALUE old, VALUE new]*field_count)`  
**DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP(3025)**  
    `args(uint32 context, uint8 success, [uint16 field_count],
         [uint16 field_id, <VALUE>]*field_count)`  
> These message replaces the current values of the given object with new values,
> but only if the 'old' values match the current state of the database.
>
> This method of updating the database is used to prevent race conditions,
> particularily when the new values are derived or dependent on the old values.
>
> If any of the given _old_ values don't match then the entire transaction fails.
> In this case, the current values of all non-empty fields will be returned after FAILURE.
>
> If any of the given fields are non-database or invalid, the entire transaction
> fails and no values are returned.


**DBSERVER_OBJECT_SET_FIELD_IF_EMPTY(3026)**  
    `args(uint32 context, uint32 do_id, uint16 field_id, <VALUE>)`  
**DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP(3027)**  
    `args(uint32 context, uint8 success, [uint16 field_id, <VALUE>])`  
> This message sets the given field if it does not currently have a value.  
> If the field is non-empty, the current value will be returned after FAILURE.


**DBSERVER_OBJECT_DELETE_FIELD(3030)**  
   `args(uint32 do_id, uint16 field_id)`  
**DBSERVER_OBJECT_DELETE_FIELDS(3031)**  
   `args(uint32 do_id, uint16 field_count, [uint16 field_id]*field_count)`  
> These messages delete individual fields from a database object.
>
> Fields with default values will be reset to the default instead.


**DBSERVER_OBJECT_DELETE(3032)**  
    `args(uint32 do_id)`  
> This message removes the object with the given do_id from the server.  
