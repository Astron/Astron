# OpenOTP project

OpenOTP is an open-source, decentralized MMORPG game server suite. The design is inspired by a similar unrelated project developed at the Disney Interactive Media Group, and used in-house from 2001 until 2013.

The game server suite consists of many components, which handle separate tasks in order to distribute the workload of managing a multi-sharded game environment with many objects and clients.

If you would like to contribute, please join us on irc. [#OpenOTP on irc.freenode.net](irc://irc.freenode.net/OpenOTP)

# Overview

## Objects
The core concept in an OpenOTP environment is a DistributedObject. These represent individual game objects that clients may know of and/or control. Every DistributedObject (or "DO" for short) contains one or more "fields" - simple properties, like the position or appearance of an object - which may be "updated" periodically. DOs are hierarchical: Each DO contains many "zones" (unsigned 32-bit integers) which can contain child DOs. This makes up the basis of the visibility system.

## Visibility system
As stated before, every DO lives beneath a parent, with the exception of the root object. If a client can see a DO, it may request to be informed of objects beneath that object within a zone. It will continue to be informed of objects (and their updates) as long as the request remains active. Such a request is called **interest**.

## DC files
DC files (short for "distributed class" files) are the core protocol description for a game built upon OpenOTP. A DC file is a pre-agreed list of dclasses (object types which may be created) and fields for each one. The ordering in a DC file is used to convert the human-readable dclass/field names to 16-bit numbers which may be efficiently sent on the network. Because all components in the OpenOTP server use the same DC file, the 16-bit IDs can be converted back to the original fields as requested.

In addition, every field in a DC file may contain one or more **keywords**. These describe the proper routing/behavior of a field update as it passes through the OpenOTP cluster.  
Some keywords are listed below:

* broadcast: Any updates on this field should be sent to all clients that can see the object.
* airecv: Any updates on this field should be sent to any AI servers that know about this object.
* ownrecv: Certain objects may be "owned" by a client. This client has special privileges for the object, and may change the object's zone at will. This keyword means that the owning client should receive any updates for this field.
* ownsend: A client is allowed to update this field if it owns the object.
* clsend: All clients that can see the object are allowed to update this field.
* db: The field contains some information that should persist long-term. If the object exists in a database, the database will automatically remember this update and reapply it when the object is loaded next.
* ram: The field contains some information that should persis short term. If the object is disabled, the information will be lost. When this object enters a client's interest, it will automatically receive the last update sent for this field.
* required: This field must be present at all times. The server will interpret this to mean that the field's value is fundamental to the existence of the object. It is therefore incorporated into all most(it needs broadcast for some) object snapshots, sent when the object is created or visible.

## Game-specific components

The OpenOTP server is only an environment for keeping track of clients and game objects. The behavior of most of these game objects is not the concern of OpenOTP. Rather, there are game-specific components that a developer wishing to use OpenOTP must implement in order to update game behavior.

The first, and most obvious, component that a developer must provide is a **client**. Clients are participants in an OpenOTP world. The client can connect to the server and create an object (usually an avatar) and has limited control over that object.
The client can then use that object to interact with the rest of the game world.

The second component is an implementation of a game's sharding logic. In OpenOTP terminology, this is known as an **AI server**. An AI server has complete control over all objects in its shard. It may create objects and perform privileged updates on them.

The third component is similar to an AI, but manages game-global objects. We call this an **UberDOG**, or UD for short. An UberDOG is allowed to create and manage DistributedObjectGlobals, which are unique in that they have no defined place within the visibility graph.  
Instead, a DistributedObjectGlobal's existence is hard-coded into all clients. There is no dynamic discovery mechanism for DOGs, nor can they have any persistent fields on them.

Each component may have its own **perspective** onto a DO. This allows different logic to exist, for the same object, for the client, AI, and UD. The perspectives are distinguished by a conventional notation:  
If the perspective is intended to represent a DO on a client, no suffix is used.  
If the perspective is intended to represent the DO on an AI or UD, the object's name is suffixed with "AI" or "UD", respectively.  
For example, if a developer wished to create an object called a DistributedMonkey, the following classes may exist in the game code:  
DistributedMonkey: The client-side representation of the object, which handles rendering and sound output.  
DistributedMoneyOV: The client-side owner-view representation of the object. It is like an ai representation of the object, except that it handles fields marked ownrecv instead of fields marked airecv.  
DistributedMonkeyAI: The AI-side representation of the object. This contains all server-side logic. For example, the monkey's movement and feeding behaviors.  
DistributedMonkeyUD: An UD-side representation of the object. For game objects like this, an UD representation is rarely needed, but can be useful if an UberDOG needs to know of certain events that happen on this game object.

For DistributedObjectGlobals, the scheme works slightly differently. A DOG is used in cases where some RPC functionality is needed from the gameserver.  
For example, if you wanted to create a mechanism where players could redeem scratch code cards for in-game currency, you might create a ScratchCardManager, which would inherit from DistributedObjectGlobal.

The ScratchCardManager would have the following two representations:  
ScratchCardManagerUD: The UD-side representation of the object. The object's doId is hard coded.  
ScratchCardManager: The client-side representation of the same. The client would be aware of this object because the object type and ID would be hard-coded into the code. The client can send updates on this object to request to redeem codes.

# OpenOTP components
Within the OpenOTP cluster, there are many components that manage individual parts of the game. Each piece is focused on doing one thing, and doing it well. Here we describe some of them in loose detail:

## Message Director
The message director receives messages from other servers, and routes them. A "message" is just an atomic blob, with a maximum size of approximately 64kB, sent from one component to another. The routing is performed by means of routing identifiers called **channels**, where a message contains any number of destination channels, and most messages include a source channel. Each component tells the MD which channels it would like to subscribe to, and receives messages sent to its subscribed channels. In this manner, the messaging architecture of OpenOTP is actually a very simple publish-subscribe system. The message director is the simplest component of OpenOTP.

## Client Agent
The client agent handles communication with the game client. Game clients do not directly communicate with OpenOTP. Rather, they communicate with the client agent, which in turn communicates with OpenOTP. Most of the security is implemented in the client agent, which enforces the clsend and ownsend keyword restrictions. For example, if a client tries to update a field that is not marked clsend, or ownsend on an object it controls, the client agent will automatically disconnect the client and log a security violation. Since the client agent may have game-specific code, OpenOTP provides a very simple reference implementation. You may want to subclass this base implementation to implement certain game-specific logic, such as allowing clients to create their own avatars directly, without relying on an UberDOG.

## State Server
The state server manages the short-term state of all DistributedObjects. It stores information such as what type of object it is, what its object ID is, and where it's located within the visibility tree. It is also responsible for persisting the value of "ram" fields. Other components may commuicate with the state server to manipulate the object, query the object's state, and query the existence of all objects in a given location.

## Database Server
The database server handles long term persistance of "db" fields. It stores these fields in a database of some sort, and can update them, or retreive their value.

## Database-State Server
This is a specialized State Server for tracking the short-term state of objects that exist in the database. A DB-SS behaves exactly the same as a State Server, however, it also listens for updates to "db"-keyworded fields and informs the database of the change.  
In addition, a DB-SS listens on the entire range of object IDs that it manages. If it sees a location update for an object in its range, it will automatically fetch the object out of the database and convert it into a state-server-object.

# Development

## The team
* **CFSworks** is the main architect of the project. His responsibilities include writing documentation and unit tests, as well as providing guidance on the organization of the code and system architecture.  
* **MMavipc** is pretty great too.

## License
The OpenOTP project is currently available under the GPL license. The terms of this license are available in the "LICENSE" file of this archive.
