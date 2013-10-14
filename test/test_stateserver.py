#!/usr/bin/env python2
import unittest
from socket import *

from common import *
from testdc import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

general:
    dc_files:
        - %r

roles:
    - type: stateserver
      control: 100
""" % test_dc

def appendMeta(datagram, doid=None, parent=None, zone=None, dclass=None):
    if doid is not None:
        datagram.add_uint32(doid)
    if parent is not None:
        datagram.add_uint32(parent)
    if zone is not None:
        datagram.add_uint32(zone)
    if dclass is not None:
        datagram.add_uint16(dclass)

def deleteObject(conn, sender, doid):
    dg = Datagram.create([doid], sender, STATESERVER_OBJECT_DELETE_RAM)
    dg.add_uint32(doid)
    conn.send(dg)

class TestStateServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

    @classmethod
    def tearDownClass(cls):
        cls.daemon.stop()

    # Tests CREATE_OBJECT_WITH_REQUIRED and OBJECT_DELETE_RAM
    def test_create_delete(self):
        ai = ChannelConnection(5000<<32|1500)
        parent = ChannelConnection(5000)

        ### Test for CreateRequired with a required value ###
        # Create a DistributedTestObject1...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 101000000, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai.send(dg)

        # The object should announce its entry to the zone-channel...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 101000000, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(ai.expect(dg))

        # The object should tell the parent its arriving...
        dg = Datagram.create([5000], 101000000, STATESERVER_OBJECT_CHANGING_LOCATION)
        appendMeta(dg, parent=5000, zone=1500) # New location
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # Old location
        self.assertTrue(parent.expect(dg))


        ### Test for DeleteRam ### (continues from previous)
        # Destroy our object...
        deleteObject(ai, 5, 101000000)

        # Object should tell its parent it is going away...
        dg = Datagram.create([5000], 101000000, STATESERVER_OBJECT_CHANGING_LOCATION)
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # New location
        appendMeta(dg, parent=5000, zone=1500) # Old location
        self.assertTrue(parent.expect(dg))

        # Object should announce its disappearance...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        self.assertTrue(ai.expect(dg))

        # We're done here...
        ### Cleanup ###
        parent.close()
        ai.close()

    # Tests the handling of the broadcast keyword by the stateserver
    def test_broadcast(self):
        ai = ChannelConnection(5000<<32|1500)

        ### Test for Broadcast to location ###
        # Create a DistributedTestObject2...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 101000005, 5000, 1500, DistributedTestObject2)
        ai.send(dg)

        # Ignore the entry message, we aren't testing that here.
        ai.flush()

        # Hit it with an update on setB2.
        dg = Datagram.create([101000005], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        ai.send(dg)

        # Object should broadcast that update
        # Note: Sender should be original sender (in this case 5). This is so AIs
        #       can see who the update ultimately comes from for e.g. an airecv/clsend.
        dg = Datagram.create([5000<<32|1500], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        self.assertTrue(ai.expect(dg))

        ### Cleanup ###
        # Delete object
        deleteObject(ai, 5, 101000005)
        ai.close()

    # Tests stateserver handling of 'airecv' keyword 
    def test_airecv(self):
        conn = ChannelConnection(5)

        ### Test for the airecv keyword ###
        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 100010, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        conn.send(dg)

        dg = Datagram.create([100010], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(5)
        conn.send(dg)

        # Ignore EnterAI message, not testing that here
        conn.flush()

        dg = Datagram.create([100010], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(100010)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        conn.send(dg)

        # Now the AI should see it...
        # Note: The ai should be a separate recipient of the same message sent
        #       to the objects location channel.
        dg = Datagram.create([ai1chan, (5000<<1500|1000)], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(100010)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.assertTrue(conn.expect(dg))


        ### Test for AI notification of object deletions ### (continues from previous)
        # Delete the object
        deleteObject(conn, 5, 100010)

        # See if the AI receives the delete.
        dg = Datagram.create([ai1chan], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(100010)
        self.assertTrue(conn.expect(dg))


        ### Cleanup ###
        conn.close()

    # Tests the messages GET_AI, SET_AI, CHANGING_AI, and ENTER_AI.
    def test_set_ai(self):
        conn = ChannelConnection(5)
        rootListener = ChannelConnection(0)

        # So we can see airecvs...
        ai1chan = 1001
        ai2chan = 2002
        ai1 = ChannelConnection(ai1chan)
        ai2 = ChannelConnection(ai2chan)

        # So we can see communications between objects...
        doid1 = 1010101
        doid2 = 2020202
        obj1 = ChannelConnection(doid1)
        obj2 = ChannelConnection(doid2)
        children1 = ChannelConnection(PARENT_PREFIX|doid1)
        children2 = ChannelConnection(PARENT_PREFIX|doid2)


        ### Test for SetAI on an object without children, AI, or optional fields ###
        # Create our first object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        conn.send(dg)

        # Object should not ask its parent (INVALID_CHANNEl) for an AI channel...
        rootListener.expect_none()

        # First object belongs to AI1...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan) # AI channel
        conn.send(dg)

        # Obj1 should announce its presence to AI1...
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(ai1.expect(dg)) # AI recieved ENTER_AI

        # ... but should not tell its children (it has none) ...
        children1.expect_none()



        ### Test for SetAI on an object with an existing AI ### (continues from previous)
        # Set AI to new AI channel
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai2chan) # AI channel
        conn.send(dg)

        # Obj1 should tell its old AI channel that it is changing AI...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid1) # Id
        dg.add_uint64(ai2chan) # New AI
        dg.add_uint64(ai1chan) # Old AI
        ai1.expect(dg)

        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai2chan], doid1, STATESERVER_OBJECT_ENTER_AI)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai2.expect(dg)



        ### Test for child AI handling on creation ### (continues from previous)
        # Let's create a second object beneath the first
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        conn.send(dg)

        # The first object is expecting two messages from the child...
        context = None
        for x in xrange(2):
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None) # Parent recieved messages
            dgi = DatagramIterator(dg)
            msgtype = dgi.read_uint16()
            # ... the second object should ask its parent for the AI channel...
            if msgtype is STATESERVER_OBJECT_GET_AI:
                self.assertTrue(dgi.matches_header([doid1], doid2, STATESERVER_OBJECT_GET_AI))
                context = dgi.read_uint32()
            # ... the second object notifies the parent it has changed location, which we ignore...
            elif msgtype is STATESERVER_OBJECT_CHANGING_LOCATION:
                continue
            else:
                self.fail("obj1 recieved unexpected msgtype: " + str(msgtype))

        # ... and the parent should reply with its AI server.
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(doid1)
        dg.add_uint64(ai2chan)
        self.assertTrue(obj2.expect(dg)) # Receiving GET_AI_RESP from parent

        # Then the second object should announce its presence to AI2...
        dg = Datagram.create([ai2chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.assertTrue(ai2.expect(dg)) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        children2.expect_none()



        ### Test for child AI handling on reparent ### (continues from previous)
        # Delete the object
        deleteObject(conn, 5, doid2)
        children2.flush() # Ignore child propogation (there shouldn't be any)
        obj1.flush() # Ignore parent notification
        ai2.flush() # Ignore AI notifcation

        # Recreate the second object with no parent
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        conn.send(dg)

        # Set the location of the second object to a zone of the first object
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid1, zone=1500)
        conn.send(dg)

        # Ignore messages about location change, not tested here
        rootListener.flush() # Won't exist anyways, but just in case
        children2.flush()

        # The first object is expecting two messages from the child...
        context = None
        for x in xrange(2):
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None) # Parent recieved messages
            dgi = DatagramIterator(dg)
            msgtype = dgi.read_uint16()
            # ... the second object should ask its parent for the AI channel...
            if msgtype is STATESERVER_OBJECT_GET_AI:
                self.assertTrue(dgi.matches_header([doid1], doid2, STATESERVER_OBJECT_GET_AI))
                context = dgi.read_uint32()
            # ... the second object notifies the parent it has changed location, which we ignore...
            elif msgtype is STATESERVER_OBJECT_CHANGING_LOCATION:
                continue
            else:
                self.fail("obj1 recieved unexpected msgtype: " + str(msgtype))

        # ... and the parent should reply with its AI server.
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(doid1)
        dg.add_uint64(ai2chan)
        self.assertTrue(obj2.expect(dg)) # Receiving GET_AI_RESP from parent

        # Then the second object should announce its presence to AI2...
        dg = Datagram.create([ai2chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.assertTrue(ai2.expect(dg)) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        children2.expect_none()



        ### Test for SetAI on an object with children ### (continues from previous)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan) # AI channel
        conn.send(dg)

        ai1expected = []
        ai2expected = []
        # Obj1 should tell its old AI channel and its children that it is changing AI...
        dg = Datagram.create([ai2chan, PARENT_PREFIX|doid1], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid1) # Id
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint64(ai2chan) # Old AI
        children1.expect(dg)
        ai2expected.append(dg)
        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai1expected.append(dg)

        # Obj2 will also tell the old AI channel that it is changing AI...
        dg = Datagram.create([ai2chan], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid2) # Id
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint64(ai2chan) # Old AI
        children2.expect_none() # It has no children
        ai2expected.append(dg)
        # ... and the new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid2, STATESERVER_OBJECT_ENTER_AI)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai1expected.append(dg)

        self.assertTrue(ai1.expect_multi(ai1expected, only=True))
        self.assertTrue(ai2.expect_multi(ai1expected, only=True))



        ### Test for AI messages with various corner cases ### (continues from previous)
        # Now let's test the verification of the AI channel notification system.

        # A notification with an incorrect parent should do nothing:
        dg = Datagram.create([doid2], 0xABABABAB, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(0xABABABAB)
        dg.add_uint64(0x17FEEF71) # New AI
        dg.add_uint64(ai1chan) # Old AI
        conn.send(dg)
        self.assertTrue(obj2.expect(dg)) # Ignore received dg
        self.assertTrue(obj2.expect_none())
        self.assertTrue(ai1.expect_none())
        self.assertTrue(obj1.expect_none())
        self.assertTrue(children2.expect_none())

        # A notification with the same AI channel should also do nothing:
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid1)
        dg.add_uint64(ai1chan)
        dg.add_uint64(ai1chan)
        obj1.send(dg)
        self.assertTrue(obj2.expect(dg)) # Ignore received dg
        self.assertTrue(obj2.expect_none())
        self.assertTrue(ai1.expect_none())
        self.assertTrue(obj1.expect_none())
        self.assertTrue(children2.expect_none())


        ### Cleanup ###
        # Delete objects
        for doid in [obj1id, obj2id, obj3id, obj4id]:
            deleteObject(conn, 5, doid)

        # Close channels
        conn.close()
        ai1.close()
        ai2.close()
        obj1.close()
        obj2.close()
        children1.close()
        children2.close()

    # Tests stateserver handling of the 'ram' keyword
    def test_ram(self):
        ai = ChannelConnection(13000<<22|6800)
        ai.add_channel(13000<<22|4800)

        ### Test that ram fields are remembered ###
        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 102000000, 13000, 6800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        ai.send(dg)

        # See if it shows up...
        dg = Datagram.create([13000<<32|6800], 102000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 102000000, 13000, 6800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        self.assertTrue(ai.expect(dg))

        # At this point we don't care about zone 6800.
        ai.remove_channel(13000<<32|6800)

        # Hit it with a RAM update.
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(102000000)
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        ai.send(dg)

        # Ignore the broadcast, not testing that here
        ai.flush()

        # Now move it over into zone 4800...
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=13000, zone=4800)
        ai.send(dg)

        # Verify that it announces its entry with the RAM field included.
        dg = Datagram.create([13000<<32|4800], 102000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, 102000000, 13000, 4800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        dg.add_uint16(1) # Other fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.assertTrue(ai.expect(dg))


        ### Cleanup ###
        deleteObject(ai, 5, 102000000)
        ai.close()

    # Tests the messages SET_LOCATION, CHANGING_LOCATION, and ENTER_LOCATION
    def test_set_location(self):
        location = ChannelConnection(14000<<32|9800)
        location.add_channel(14000<<32|9900)

        ### Test for a call to SetLocation ###
        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 105000000, 14000, 9800, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        location.send(dg)

        # See if it shows up...
        dg = Datagram.create([14000<<32|9800], 105000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 105000000, 14000, 9800, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        self.assertTrue(location.expect(dg))

        # Now move it over into zone 9900...
        dg = Datagram.create([105000000], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=14000, zone=9900)
        location.send(dg)

        # See if it announces its departure from 9800...
        expected = []
        dg = Datagram.create([14000<<32|9800], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(105000000) # ID
        appendMeta(dg, parent=14000, zone=9900) # New location
        appendMeta(dg, parent=14000, zone=9800) # Old location
        expected.append(dg)
        # ...and its entry into 9900.
        dg = Datagram.create([14000<<32|9900], 105000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 105000000, 14000, 9900, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        expected.append(dg)

        self.assertTrue(location.expect_multi(expected, only=True))



        #### TODO: Test more corner cases!!! See test_set_ai for inspiration ####



        ### Clean up ###
        deleteObject(location, 5, 105000000)
        location.close()



    #### TODO: Messages from here down have not been reviewed/updated and will fail with errors. ####

    def test_inheritance(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(67000<<32|2000))

        # Create a DTO3, which inherits from DTO1.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(67000) # Parent
        dg.add_uint32(2000) # Zone
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(110000000) # ID
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        self.c.send(dg)

        # Does it show up right?
        dg = Datagram.create([67000<<32|2000], 110000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(67000) # Parent
        dg.add_uint32(2000) # Zone
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(110000000) # ID
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        self.assertTrue(self.c.expect(dg))

        # Cool, now let's test the broadcast messages:
        for field in [setRDB3, setRequired1]:
            dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_uint32(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            self.c.send(dg)
            dg = Datagram.create([67000<<32|2000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_uint32(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            self.assertTrue(self.c.expect(dg))

        # This message is NOT part of DTO1/3 and should fail.
        # Bonus points for logging an ERROR log.
        dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(110000000)
        dg.add_uint16(setB2)
        dg.add_uint32(0x11225533)
        self.c.send(dg)
        self.assertTrue(self.c.expect_none())

        # Clean up.
        self.c.send(Datagram.create_remove_channel(67000<<32|2000))
        dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(110000000)
        self.c.send(dg)

    def test_error(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(80000<<32|1234))

        # Create a regular object, this is not an error...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(234000000) # ID
        dg.add_uint32(819442) # setRequired1
        self.c.send(dg)

        # The object should announce its entry to the zone-channel...
        dg = Datagram.create([80000<<32|1234], 234000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(234000000) # ID
        dg.add_uint32(819442) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Send it an update on a bad field...
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(234000000)
        dg.add_uint16(0x1337)
        dg.add_uint32(0)
        self.c.send(dg)

        # Nothing should happen and the SS should log an error.
        self.assertTrue(self.c.expect_none())

        # How about a truncated update on a valid field?
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(234000000)
        dg.add_uint16(setRequired1)
        dg.add_uint16(0) # Whoops, 16-bit instead of 32-bit!
        self.c.send(dg)

        # Nothing should happen + error.
        self.assertTrue(self.c.expect_none())

        # Let's try creating it again.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(234000000) # ID
        dg.add_uint32(1234567) # setRequired1
        self.c.send(dg)

        # NOTHING SHOULD HAPPEN - additionally, the SS should log either an
        # error or a warning
        self.assertTrue(self.c.expect_none())

        # Let's try making another one, but we'll forget the setRequired1.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(235000000) # ID
        self.c.send(dg)

        # Once again, nothing should happen and the SS should log an ERROR.
        self.assertTrue(self.c.expect_none())

        # Let's try making a bad object.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(0x1337)
        dg.add_uint32(236000000) # ID
        self.c.send(dg)

        # Nothing should happen and the SS should log an ERROR.
        self.assertTrue(self.c.expect_none())

        # Clean up.
        self.c.send(Datagram.create_remove_channel(80000<<32|1234))
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(234000000)
        self.c.send(dg)
        dg = Datagram.create([235000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(235000000)
        self.c.send(dg)

    def test_create_with_other(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(90000<<32|4321))

        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(4321) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(545630000) # ID
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        self.c.send(dg)

        # We should get an ENTERZONE_WITH_REQUIRED_OTHER...
        dg = Datagram.create([90000<<32|4321], 545630000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(4321) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(545630000) # ID
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        self.assertTrue(self.c.expect(dg))

        # If we try to include a non-ram as OTHER...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(4321) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(545640000) # ID
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setB1)
        dg.add_uint8(42)
        self.c.send(dg)

        # ...the object should show up, but without the non-RAM field.
        # Additionally, an ERROR should be logged.
        dg = Datagram.create([90000<<32|4321], 545640000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(4321) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(545640000) # ID
        dg.add_uint32(2099) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        dg = Datagram.create([545630000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(545630000)
        self.c.send(dg)
        dg = Datagram.create([545640000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(545640000)
        self.c.send(dg)
        self.c.send(Datagram.create_remove_channel(90000<<32|4321))

    def test_locate(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(0x4a03))

        # Throw a few objects out there...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(48000) # Parent
        dg.add_uint32(624800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(583312) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(223315) # Parent
        dg.add_uint32(61444444) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(583311) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(51792) # Parent
        dg.add_uint32(5858182) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(583310) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)

        # Now let's go on a treasure hunt:
        for obj, parent, zone in [
            (583312, 48000, 624800),
            (583311, 223315, 61444444),
            (583310, 51792, 5858182)]:
            context = (obj-500000)*2838
            dg = Datagram.create([obj], 0x4a03, STATESERVER_OBJECT_GET_LOCATION)
            dg.add_uint32(context)
            self.c.send(dg)

            dg = Datagram.create([0x4a03], obj, STATESERVER_OBJECT_GET_LOCATION_RESP)
            dg.add_uint32(context)
            dg.add_uint32(obj)
            dg.add_uint32(parent)
            dg.add_uint32(zone)
            self.assertTrue(self.c.expect(dg))

            # Found the object! Now clean it up.
            dg = Datagram.create([obj], 5, STATESERVER_OBJECT_DELETE_RAM)
            dg.add_uint32(obj)
            self.c.send(dg)

        self.c.send(Datagram.create_remove_channel(0x4a03))

    def test_shard_reset(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(4030<<32|201))
        self.c.send(Datagram.create_add_channel(4030<<32|202))
        self.c.send(Datagram.create_add_channel(45543<<32|6868))
        self.c.send(Datagram.create_add_channel(201<<32|4444))

        # Create a pair of objects...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(45543) # Parent
        dg.add_uint32(6868) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(201) # ID
        dg.add_uint32(6789) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(201) # Parent
        dg.add_uint32(4444) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(202) # ID
        dg.add_uint32(6789) # setRequired1
        self.c.send(dg)

        # Parent has an AI channel...
        dg = Datagram.create([201], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint32(201)
        dg.add_uint64(31337)
        self.c.send(dg)

        # The notifications trickle down to the children; disregard.
        self.c.flush()

        # Now let's try hitting the SS with a reset for the wrong AI:
        dg = Datagram.create([100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(41337)
        self.c.send(dg)

        # Nothing should happen:
        self.assertTrue(self.c.expect_none())

        # Now the right AI:
        dg = Datagram.create([100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(31337)
        self.c.send(dg)

        expected = []
        # The parent should relay down to its children:
        dg = Datagram.create([4030<<32|201], 201, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(31337)
        expected.append(dg)
        # ...which should relay down to its children:
        dg = Datagram.create([4030<<32|202], 202, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(31337)
        expected.append(dg)
        # Then both objects should die:
        dg = Datagram.create([45543<<32|6868], 201, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(201)
        expected.append(dg)
        dg = Datagram.create([201<<32|4444], 202, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(202)
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(4030<<32|201))
        self.c.send(Datagram.create_remove_channel(4030<<32|202))
        self.c.send(Datagram.create_remove_channel(45543<<32|6868))
        self.c.send(Datagram.create_remove_channel(201<<32|4444))

    def test_query(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(890))

        # Make a few objects...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(4) # Parent
        dg.add_uint32(2) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(15000) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483312) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483311) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(51792) # Parent
        dg.add_uint32(5858182) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483310) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)

        # Query each one out of the SS:
        dg = Datagram.create([483310, 483311, 483312], 890, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0x600DF00D)
        self.c.send(dg)

        # The SS is a very efficient organization and will return everything
        # promptly.
        expected = []
        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x600DF00D)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483312) # ID
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # No extra fields
        expected.append(dg)
        dg = Datagram.create([890], 483311, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x600DF00D)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483311) # ID
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # No extra fields
        expected.append(dg)
        dg = Datagram.create([890], 483310, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x600DF00D)
        dg.add_uint32(51792) # Parent
        dg.add_uint32(5858182) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483310) # ID
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # No extra fields
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Test zone queries...
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_ZONES_OBJECTS)
        dg.add_uint32(15000) # Parent
        dg.add_uint16(1) # Only looking at one zone...
        dg.add_uint32(1337) # Zone
        self.c.send(dg)

        # We should get ENTERZONEs for both objects in that zone...
        expected = []
        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483312) # ID
        dg.add_uint32(0) # setRequired1
        expected.append(dg)
        dg = Datagram.create([890], 483311, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(15000) # Parent
        dg.add_uint32(1337) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(483311) # ID
        dg.add_uint32(0) # setRequired1
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Next we should get the QUERY_ZONE_ALL_DONE from the parent...
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(15000) # Parent
        dg.add_uint16(1) # Only looking at one zone...
        dg.add_uint32(1337) # Zone
        self.assertTrue(self.c.expect(dg))
        # AND NOTHING MORE:
        self.assertTrue(self.c.expect_none())

        # Now test field queries...
        dg = Datagram.create([483312], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(483312) # ID
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xBAB55EED)
        self.c.send(dg)

        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(483312)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint8(1)
        dg.add_uint32(0)
        self.assertTrue(self.c.expect(dg))

        # Query a field not present on the object...
        dg = Datagram.create([483312], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(483312) # ID
        dg.add_uint16(setRDB3)
        dg.add_uint32(0xBAB55EED)
        self.c.send(dg)

        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(483312)
        dg.add_uint16(setRDB3)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint8(0)
        self.assertTrue(self.c.expect(dg))

        # Now let's try a QUERY_FIELDS.
        dg = Datagram.create([483312], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(483312)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint16(setRequired1)
        dg.add_uint16(setRequired1)
        dg.add_uint16(setRequired1)
        self.c.send(dg)

        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(483312)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint8(1)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        self.assertTrue(self.c.expect(dg))

        # Now a QUERY_FIELDS including a nonpresent field.
        dg = Datagram.create([483312], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(483312)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint16(setRequired1)
        dg.add_uint16(setRDB3)
        self.c.send(dg)

        dg = Datagram.create([890], 483312, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(483312)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint8(0)
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(890))
        dg = Datagram.create([15000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(15000)
        self.c.send(dg)
        dg = Datagram.create([483310], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(483310)
        self.c.send(dg)
        dg = Datagram.create([483311], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(483311)
        self.c.send(dg)
        dg = Datagram.create([483312], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(483312)
        self.c.send(dg)

    def test_update_multiple(self):
        self.c.send(Datagram.create_add_channel(5985858))
        self.c.send(Datagram.create_add_channel(12512<<32|66161))

        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(12512) # Parent
        dg.add_uint32(66161) # Zone
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(55555) # ID
        dg.add_uint32(0) # setRequired1
        dg.add_uint32(0) # setRDB3
        self.c.send(dg)

        # Ignore the entry message...
        self.c.flush()

        # Send a multi-field update with two broadcast fields and one ram.
        dg = Datagram.create([55555], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_uint32(55555)
        dg.add_uint16(3) # 3 fields:
        dg.add_uint16(setDb3)
        dg.add_string('Time is candy')
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        self.c.send(dg)

        # TODO: Reenable this test once atomic updates are implemented.
        # Verify that the broadcasts go out, in order...
        dg = Datagram.create([12512<<32|66161], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_uint32(55555)
        dg.add_uint16(2) # 3 fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        #self.assertTrue(self.c.expect(dg))
        self.c.flush()

        # Query the ram/required...
        dg = Datagram.create([55555], 5985858, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0)
        self.c.send(dg)

        # See if those updates have properly gone through...
        dg = Datagram.create([5985858], 55555, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0)
        dg.add_uint32(12512) # Parent
        dg.add_uint32(66161) # Zone
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(55555) # ID
        dg.add_uint32(0xD00D) # setRequired1
        dg.add_uint32(0) # setRDB3
        dg.add_uint16(1) # 1 extra field:
        dg.add_uint16(setDb3)
        dg.add_string('Time is candy')
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(5985858))
        self.c.send(Datagram.create_remove_channel(12512<<32|66161))
        dg = Datagram.create([55555], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(55555)
        self.c.send(dg)

    def test_ownrecv(self):
        self.c.flush()
        self.c.send(Datagram.create_add_channel(14253647))

        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(88833) # Parent
        dg.add_uint32(99922) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(74635241) # ID
        dg.add_uint32(0) # setRequired1
        self.c.send(dg)

        # Set the owner channel:
        dg = Datagram.create([74635241], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(14253647)
        self.c.send(dg)

        # See if it enters OWNER_RECV.
        dg = Datagram.create([14253647], 74635241, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_uint32(88833) # Parent
        dg.add_uint32(99922) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(74635241) # ID
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # Number of OTHER fields.
        self.assertTrue(self.c.expect(dg))

        # Send an ownrecv message...
        dg = Datagram.create([74635241], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(74635241)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        self.c.send(dg)

        # See if our owner channel gets it.
        dg = Datagram.create([14253647], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(74635241)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        self.assertTrue(self.c.expect(dg))

        # Change away...
        dg = Datagram.create([74635241], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(197519725497)
        self.c.send(dg)

        # See if we get a CHANGE_OWNER_RECV
        dg = Datagram.create([14253647], 5, STATESERVER_OBJECT_CHANGING_OWNER)
        dg.add_uint32(74635241)
        dg.add_uint64(197519725497)
        dg.add_uint64(14253647)
        self.assertTrue(self.c.expect(dg))

        # Switch our channel subscription to follow it.
        self.c.send(Datagram.create_add_channel(197519725497))
        self.c.send(Datagram.create_remove_channel(14253647))

        # Hit it with a delete.
        dg = Datagram.create([74635241], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(74635241)
        self.c.send(dg)

        # Make sure the owner is notified.
        dg = Datagram.create([197519725497], 74635241, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(74635241)
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(197519725497))

    def test_molecular(self):
        self.c.flush()
        self.c.send(Datagram.create_add_channel(13371337))
        self.c.send(Datagram.create_add_channel(88<<32|99))

        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(88) # Parent
        dg.add_uint32(99) # Zone
        dg.add_uint16(DistributedTestObject4)
        dg.add_uint32(73317331) # ID
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        self.c.send(dg)

        # Verify its entry...
        dg = Datagram.create([88<<32|99], 73317331, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(88) # Parent
        dg.add_uint32(99) # Zone
        dg.add_uint16(DistributedTestObject4)
        dg.add_uint32(73317331) # ID
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        self.assertTrue(self.c.expect(dg))

        # Send a molecular update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        self.c.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<32|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        self.assertTrue(self.c.expect(dg))

        # Look at the object and see if the requireds are updated...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0)
        self.c.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0)
        dg.add_uint32(88) # Parent
        dg.add_uint32(99) # Zone
        dg.add_uint16(DistributedTestObject4)
        dg.add_uint32(73317331) # ID
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(77) # setZ
        dg.add_uint16(0) # 0 extra fields.
        self.assertTrue(self.c.expect(dg))

        # Now try a RAM update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        self.c.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<32|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        self.assertTrue(self.c.expect(dg))

        # A query should have all of the individual fields, not the molecular.
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0)
        self.c.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0)
        dg.add_uint32(88) # Parent
        dg.add_uint32(99) # Zone
        dg.add_uint16(DistributedTestObject4)
        dg.add_uint32(73317331) # ID
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(77) # setZ
        dg.add_uint16(3) # 3 extra fields:
        dg.add_uint16(setOne)
        dg.add_uint8(1)
        dg.add_uint16(setTwo)
        dg.add_uint8(2)
        dg.add_uint16(setThree)
        dg.add_uint8(3)
        self.assertTrue(self.c.expect(dg))

        # Now let's test querying individual fields...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(73317331)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint16(setXyz)
        dg.add_uint16(setOne)
        dg.add_uint16(setUnrelated)
        dg.add_uint16(set123)
        dg.add_uint16(setX)
        self.c.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(73317331)
        dg.add_uint32(0xBAB55EED)
        dg.add_uint8(1)
        dg.add_uint16(setXyz)
        dg.add_uint32(55)
        dg.add_uint32(66)
        dg.add_uint32(77)
        dg.add_uint16(setOne)
        dg.add_uint8(1)
        dg.add_uint16(setUnrelated)
        dg.add_uint32(999999)
        dg.add_uint16(set123)
        dg.add_uint8(1)
        dg.add_uint8(2)
        dg.add_uint8(3)
        dg.add_uint16(setX)
        dg.add_uint32(55)
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(73317331)
        self.c.send(dg)
        self.c.send(Datagram.create_remove_channel(13371337))
        self.c.send(Datagram.create_remove_channel(88<<32|99))

if __name__ == '__main__':
    unittest.main()
