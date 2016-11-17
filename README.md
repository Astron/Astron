Astron Project
--------------
_A Server Technology for Realtime Object Networking_

Astron is an open-source, distributed server suite particularly well-suited for powering MMO games.
The design is inspired by a similar unrelated project developed at the Disney Interactive Media
Group, and used in-house from 2001 until 2013.

The suite consists of many components, which handle separate tasks in order to distribute
the workload of managing a multi-sharded game/application environment with many objects and clients.

[![Build Status](https://travis-ci.org/Astron/Astron.svg?branch=master)](https://travis-ci.org/Astron/Astron)

## Overview ##

### Objects ###
The core concept in an Astron environment is a DistributedObject. These represent individual game
objects that clients may know of and/or control. Every DistributedObject (or "DO" for short) contains
one or more "fields" - simple properties, like the position or appearance of an object - which may be
"updated" periodically. DOs are hierarchical: Each DO contains many "zones" (unsigned 32-bit integers)
which can contain child DOs. This makes up the basis of the visibility system.

### Visibility System ###
As stated before, every DO lives beneath a parent, with the exception of the root object. If a client
can see a DO, it may request to be informed of objects beneath that object within a zone.
It will continue to be informed of objects (and their updates) as long as the request remains active.
Such a request is called **interest**.

### DC Files ###
DC files (short for "distributed class" files) are the core protocol description for a game built
upon Astron. A DC file is a pre-agreed list of dclasses (object types which may be created) and
fields for each one. The ordering in a DC file is used to convert the human-readable dclass/field
names to 16-bit numbers which may be efficiently sent on the network. Because all components in the
Astron server use the same DC file, the 16-bit IDs can be converted back to the original fields as
requested.

In addition, every field in a DC file may contain one or more **keywords**. These describe the proper
routing/behavior of a field update as it passes through the Astron cluster.  
Some keywords are listed below:

> ### Persistence ###
>
> **ram**  
> The field contains some information that should persist short term.
> If the object is disabled, the information will be lost.
>
> **required** _implies **ram**_  
> This field must be present at all times. The server will interpret this to mean that the
> field's value is fundamental to the existence of the object.  Therefore, an object must be given
> required values on creation, and required values will reset to defaults when individually cleared.
> The field is included in all object snapshots to the AI, and all snapshots to the client if it is
> also visible to the client through ownrecv or clrecv.
>
> **db**  
> The field contains some information that should persist long-term. If the object exists in a
> database, the database will remember updates to this field and restore them on object activation.
>
> ### Publication | Subscription ###
>
> **airecv**  
> Any updates on this field should be sent to the object's managing AI.
>
> **ownrecv**  
> Objects may be "owned" by a client.
> This owner may change the object's parent and zone at will (unless disabled by the ClientAgent).
> The owner should receive this field when it gains ownership of the object.
> Additionally, any updates on this field should be sent to the object's owner.
>
> **clrecv**  
> The client should receive this field when the object becomes visible to the client.
>
> **broadcast** _implies **clrecv**_  
> Any updates on this field should be sent to all clients that can see the object.
>
> **ownsend**  
> Objects may be "owned" by a client.
> A client is allowed to update this field if it owns the object.
>
> **clsend**  
> All clients that can see the object are allowed to update this field.



## Application-specific components ##

The Astron server is only an environment for keeping track of clients and game objects. The behavior
of most of these game objects is not the concern of Astron. Rather, there are application-specific
components that a developer wishing to use Astron must implement in order to update game behavior.

The first, and most obvious, component that a developer must provide is a **client**. Clients are
participants in an Astron world. The client can connect to the server and communicate with objects
(in games: usually an avatar) and has limited control over those objects.
The client can then use that object to interact with the rest of the application.

The second component is an implementation of the applications's sharding logic. In Astron terminology,
this is known as an **AI server**. An AI server has complete control over all objects in its shard.
It may create objects and perform privileged updates on them.

The third component is similar to an AI, but manages game-global objects. We call this an **UberDOG**,
or UD for short. An UberDOG is allowed to create and manage DistributedObjectGlobals, which are
unique in that they have no defined place within the visibility graph.  
Instead, a DistributedObjectGlobal's existence is hard-coded into all clients. There is no dynamic
discovery mechanism for DOGs, nor can they have any persistent fields on them.  

DOGs are primarily used for RPC-like operations.  For most Distributed Objects, interaction is prohibited
to unauthenticated clients.  UberDOGs can be additionally configured to allow these clients to interact with
them. It is typical to use such an UberDOG to handle Authentication, but it can also be used to provide a
public RPC-based API to anonymous users.

Each component may have its own **perspective** onto a DO. This allows different logic to exist, for
the same object, for the client, AI, and UD. The perspectives are distinguished by a conventional notation:  
If the perspective is intended to represent a DO on a client, no suffix is used.  
If the perspective is intended to represent the DO on an AI or UD, the object's name is suffixed with "AI" or "UD", respectively.  
For example, if a developer wished to create an object called a DistributedMonkey, the following classes may exist in the game code:  
DistributedMonkey: The client-side representation of the object, which handles rendering and sound output.  
DistributedMonkeyOV: The client-side owner-view representation of the object. It is like an ai representation of the object, except that it handles fields marked ownrecv instead of fields marked airecv.  
DistributedMonkeyAI: The AI-side representation of the object. This contains all server-side logic. For example, the monkey's movement and feeding behaviors.  
DistributedMonkeyUD: An UD-side representation of the object. For game objects like this, an UD representation is rarely needed, but can be useful if an UberDOG needs to know of certain events that happen on this game object.

For DistributedObjectGlobals, the scheme works slightly differently. A DOG is used in cases where some RPC functionality is needed from the gameserver.  
For example, if you wanted to create a mechanism where players could redeem scratch code cards for in-game currency, you might create a ScratchCardManager, which would inherit from DistributedObjectGlobal.

The ScratchCardManager would have the following two representations:  
ScratchCardManagerUD: The UD-side representation of the object. The object's doId is hard coded.  
ScratchCardManager: The client-side representation of the same. The client would be aware of this object because the object type and ID would be hard-coded into the code. The client can send updates on this object to request to redeem codes.



## Astron Roles ##
Within the Astron cluster, Astron daemons are configured to serve certain roles in the cluster. Astron daemons may serve one or more roles. Here we describe some of them in loose detail:

### Message Director ###
The message director receives messages from other daemons, and routes them. A "message" is just an atomic blob, with a maximum size of approximately 64kB, sent from one daemon to another. The routing is performed by means of routing identifiers called **channels**, where a message contains any number of destination channels, and most messages include a source channel. Each component tells the MD which channels it would like to subscribe to, and receives messages sent to its subscribed channels. In this manner, the messaging architecture of Astron is actually a very simple publish-subscribe system. The message director is the simplest component of Astron.

### Client Agent ###
The client agent handles communication with the game client. Game clients do not directly communicate with Astron. Rather, they communicate with the client agent, which in turn communicates with Astron. Most of the security is implemented in the client agent, which enforces the clsend and ownsend keyword restrictions. For example, if a client tries to update a field that is not marked clsend, or ownsend on an object it controls, the client agent will automatically disconnect the client and log a security violation. Since the client agent may have game-specific code, Astron provides a very simple reference implementation. You may want to subclass this base implementation to implement certain game-specific logic, such as allowing clients to create their own avatars directly, without relying on an UberDOG.

### State Server ###
The state server manages the short-term state of all DistributedObjects. It stores information such as what type of object it is, what its object ID is, and where it's located within the visibility tree. It is also responsible for persisting the value of "ram" fields. Other components may communicate with the state server to manipulate the object, query the object's state, and query the existence of all objects in a given location.

### Database Server ###
The database server handles long term persistence of "db" fields. It stores these fields in a database of some sort, and can update them, or retrieve their value.

### Database-State Server ###
This is a specialized State Server for tracking the short-term state of objects that exist in the database. A DB-SS behaves exactly the same as a State Server, however, it also listens for updates to "db"-keyworded fields and informs the database of the change.  
In addition, a DB-SS listens on the entire range of object IDs that it manages. If it sees a location update for an object in its range, it will automatically fetch the object out of the database and convert it into a state-server-object.



## Building ##
See the [build instructions](docs/building/build-readme.md).



## Development ##

### The Team ###
* **@Kestred** Kevin Stenerson is the project maintainer and one of its architects. He does Astron maintenance (bugfixes, binary-distributions, etc), design revisions, new feature planning & implementation, architecture scalability, and application/network interface cleanliness.
* **@CFSworks** Sam Edwards is the project author and the original and continuing architect. He provides guidance on the organization of the code and system architecture. He also works on major rewrites, architecture scalability, bugfixes, and efficiency improvements.
* **@AlJaMa** Alex Mault is a bouncing board for architectural design and problem solving.  He works with OS X support, feature brainstorming, and some libastron design and implementation.

### Major Contributors ###
* **@MMavipc** Maverick Mosher wrote a great deal of code to help get Astron out of its infancy.
* **@bobbybee** Alyssa Rosenzweig solves outstanding issues, bug fixes, and implements miscellaneous features.
* **@jjkoletar** Jeremy Koletar was key in Astron's early inspiration and initial design; he also works on bug fixes.

### License ###
The Astron project is currently available under the Modified BSD license (BSD 3-Clause). The terms of this license are available in the "LICENSE.md" file of this archive.

### Contributing ###
We welcome any potential contributors! Don't just start coding though; we all talk about what we're doing, what is next, etc. on IRC.
Please come in and tell us what you'd like to do, or ask what we could use help on.

#### Join us at: [#Astron on irc.freenode.net](irc://irc.freenode.net/Astron)


#### OTP Architecture resources ####
There are a lot of resources we use as a guide and for inspiration while building Astron.  New contributors might find them to be very informative, both about how the server works and in thinking about what direction Astron wants to go in.  
**NOTE** - These materials are about the original OTP server at Disney, and only used for inspiration here.

Video lectures from Disney:
 - [DistributedObjects](http://www.youtube.com/watch?v=JsgCFVpXQtQ)
 - [DistributedObjects and the OTP Server](http://www.youtube.com/watch?v=r_ZP9SInPcs)
 - [OTP Server Internals](http://www.youtube.com/watch?v=SzybRdxjYoA)

Presentation Slides
 - [MMO 101 - Building Disney's Server](http://twvideo01.ubm-us.net/o1/vault/gdconline10/slides/11516-MMO_101_Building_Disneys_Sever.pdf)

Other Documentation
 - [Building a MMG for the Million - Disney's Toontown](http://dl.acm.org/citation.cfm?id=950566.950589)
