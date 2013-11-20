#!/usr/bin/env python2
import unittest, time
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

def connect(channel):
    return ChannelConnection('127.0.0.1', 57123, channel)

def appendMeta(datagram, doid=None, parent=None, zone=None, dclass=None):
    if doid is not None:
        datagram.add_uint32(doid)
    if parent is not None:
        datagram.add_uint32(parent)
    if zone is not None:
        datagram.add_uint32(zone)
    if dclass is not None:
        datagram.add_uint16(dclass)

def createEmptyDTO1(conn, sender, doid, parent=0, zone=0, required1=0):
    dg = Datagram.create([100], sender, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
    appendMeta(dg, doid, parent, zone, DistributedTestObject1)
    dg.add_uint32(required1)
    conn.send(dg)

def deleteObject(conn, sender, doid):
    dg = Datagram.create([doid], sender, STATESERVER_OBJECT_DELETE_RAM)
    dg.add_uint32(doid)
    conn.send(dg)

CONN_POOL_SIZE = 8
class TestStateServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        cls.conn_pool = []
        cls.conn_used = []
        for x in xrange(CONN_POOL_SIZE):
            conn = connect(0)
            conn.remove_channel(0)
            cls.conn_pool.append(conn)

    @classmethod
    def tearDownClass(cls):
        for conn in cls.conn_used:
            cls.disconnect(conn)
        for conn in cls.conn_pool:
            conn.close()
        cls.daemon.stop()

    @classmethod
    def connect(cls, channel):
        conn = cls.conn_pool.pop()
        conn.add_channel(channel)
        cls.conn_used.append(conn)
        return conn

    @classmethod
    def disconnect(cls, conn):
        conn.clear_channels()
        conn.flush()
        cls.conn_used.remove(conn)
        cls.conn_pool.append(conn)

    def flush_failed(self):
        for conn in self.conn_used:
            self.disconnect(conn)

    # Tests CREATE_OBJECT_WITH_REQUIRED and OBJECT_DELETE_RAM
    def test_create_delete(self):
        self.flush_failed()
        ai = self.connect(5000<<32|1500)
        parent = self.connect(5000)

        # Repeat tests twice to verify doids can be reused
        for x in xrange(2):

            ### Test for CreateRequired with a required value ###
            # Create a DistributedTestObject1...
            createEmptyDTO1(ai, 5, 101000000, 5000, 1500, 6789)

            # The parent is expecting two messages...
            received = False
            for x in xrange(2):
                dg = parent.recv_maybe()
                self.assertTrue(dg is not None, msg="Parent did not receive ChangingLocation and/or GetAI")
                dgi = DatagramIterator(dg)
                msgtype = dgi.read_uint16()
                # The object should tell the parent its arriving...
                if dgi.matches_header([5000], 5, STATESERVER_OBJECT_CHANGING_LOCATION):
                    received = True
                    self.assertEquals(dgi.read_uint32(), 101000000) # Id
                    self.assertEquals(dgi.read_uint32(), 5000) # New parent
                    self.assertEquals(dgi.read_uint32(), 1500) # New zone
                    self.assertEquals(dgi.read_uint32(), INVALID_DO_ID) # Old parent
                    self.assertEquals(dgi.read_uint32(), INVALID_ZONE) # Old zone
                # .. and ask it for its AI, which we're not testing here and can ignore
                elif dgi.matches_header([5000], 101000000, STATESERVER_OBJECT_GET_AI):
                    continue
                else:
                    self.fail("Received header did not match expected for msgtype: " + str(msgtype))
            self.assertTrue(received) # Parent received ChangingLocation

            # The object should announce its entry to the zone-channel...
            dg = Datagram.create([5000<<32|1500], 101000000,
                                 STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
            appendMeta(dg, 101000000, 5000, 1500, DistributedTestObject1)
            dg.add_uint32(6789) # setRequired1
            self.assertTrue(*ai.expect(dg))


            ### Test for DeleteRam ### (continues from previous)
            # Destroy our object...
            deleteObject(ai, 5, 101000000)

            # Object should tell its parent it is going away...
            dg = Datagram.create([5000], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
            dg.add_uint32(101000000)
            appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # New location
            appendMeta(dg, parent=5000, zone=1500) # Old location
            self.assertTrue(*parent.expect(dg))

            # Object should announce its disappearance...
            dg = Datagram.create([5000<<32|1500], 5, STATESERVER_OBJECT_DELETE_RAM)
            dg.add_uint32(101000000)
            self.assertTrue(*ai.expect(dg))

        # We're done here...
        ### Cleanup ###
        self.disconnect(parent)
        self.disconnect(ai)

    # Tests the handling of the broadcast keyword by the stateserver
    def test_broadcast(self):
        self.flush_failed()
        ai = self.connect(5000<<32|1500)

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
        self.assertTrue(*ai.expect(dg))

        ### Cleanup ###
        # Delete object
        deleteObject(ai, 5, 101000005)
        self.disconnect(ai)

    # Tests stateserver handling of 'airecv' keyword 
    def test_airecv(self):
        self.flush_failed()
        conn = self.connect(5)
        conn.add_channel(1300)

        ### Test for the airecv keyword ###
        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 100010, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        conn.send(dg)

        dg = Datagram.create([100010], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(1300)
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
        dg = Datagram.create([1300, (5000<<32|1500)], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(100010)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.assertTrue(*conn.expect(dg))


        ### Test for AI notification of object deletions ### (continues from previous)
        # Delete the object
        deleteObject(conn, 5, 100010)

        # See if the AI receives the delete.
        dg = Datagram.create([1300, (5000<<32|1500)], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(100010)
        self.assertTrue(*conn.expect(dg))

        ### Cleanup ###
        self.disconnect(conn)

    # Tests the messages GET_AI, SET_AI, CHANGING_AI, and ENTER_AI.
    def test_set_ai(self):
        self.flush_failed()
        conn = self.connect(5)
        conn.add_channel(0)

        # So we can see airecvs...
        ai1chan = 1011
        ai2chan = 2202
        ai1 = self.connect(ai1chan)
        ai2 = self.connect(ai2chan)

        # So we can see communications between objects...
        doid1 = 1010101
        doid2 = 2020202
        obj1 = self.connect(doid1)
        obj2 = self.connect(doid2)
        children1 = self.connect(PARENT_PREFIX|doid1)
        children2 = self.connect(PARENT_PREFIX|doid2)


        ### Test for SetAI on an object without children, AI, or optional fields ###
        # Create our first object...
        createEmptyDTO1(conn, 5, doid1, 0, 0, 6789)

        # Object should not ask its parent (INVALID_CHANNEl) for an AI channel...
        self.assertTrue(conn.expect_none())

        # First object belongs to AI1...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan) # AI channel
        conn.send(dg)
        obj1.flush()

        # Obj1 should announce its presence to AI1...
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(*ai1.expect(dg)) # AI recieved ENTER_AI

        # ... but should not tell its children (it has none) ...
        self.assertTrue(children1.expect_none())



        ### Test for SetAI on an object with an existing AI ### (continues from previous)
        # Set AI to new AI channel
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai2chan) # AI channel
        conn.send(dg)
        obj1.flush()

        # Obj1 should tell its old AI channel that it is changing AI...
        dg = Datagram.create([ai1chan], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid1) # Id
        dg.add_uint64(ai2chan) # New AI
        dg.add_uint64(ai1chan) # Old AI
        self.assertTrue(*ai1.expect(dg))

        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai2chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(*ai2.expect(dg))



        ### Test for child AI handling on creation ### (continues from previous)
        # Let's create a second object beneath the first
        createEmptyDTO1(conn, 5, doid2, doid1, 1500, 1337)

        # The first object is expecting two messages from the child...
        context = None
        for x in xrange(2):
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None, "Parent did not receive ChangingLocation and/or GetAI")
            dgi = DatagramIterator(dg)
            # ... the second object should ask its parent for the AI channel...
            if dgi.matches_header([doid1], doid2, STATESERVER_OBJECT_GET_AI):
                context = dgi.read_uint32()
            # ... the second object notifies the parent it has changed location, which we ignore...
            elif dgi.matches_header([doid1], 5, STATESERVER_OBJECT_CHANGING_LOCATION):
                continue
            else:
                self.fail("Received unexpected or non-matching header.")

        # ... and the parent should reply with its AI server.
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(doid1)
        dg.add_uint64(ai2chan)
        self.assertTrue(*obj2.expect(dg)) # Receiving GET_AI_RESP from parent

        # Then the second object should announce its presence to AI2...
        dg = Datagram.create([ai2chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.assertTrue(*ai2.expect(dg)) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        self.assertTrue(children2.expect_none())



        ### Test for child AI handling on reparent ### (continues from previous)
        # Delete the object
        deleteObject(conn, 5, doid2)
        children2.flush() # Ignore child propogation (there shouldn't be any)
        obj1.flush() # Ignore parent notification
        obj2.flush() # Ignore received message
        ai2.flush() # Ignore AI notifcation

        # Recreate the second object with no parent
        createEmptyDTO1(conn, 5, doid2, 0, 0, 1337)

        # Set the location of the second object to a zone of the first object
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid1, zone=1500)
        conn.send(dg)
        obj2.flush()

        # Ignore messages about location change, not tested here
        conn.flush() # Won't exist anyways, but just in case
        children2.flush()

        # The first object is expecting two messages from the child...
        context = None
        for x in xrange(2):
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None, "Parent did not receive ChangingLocation and/or GetAI")
            dgi = DatagramIterator(dg)
            # ... the second object should ask its parent for the AI channel...
            if dgi.matches_header([doid1], doid2, STATESERVER_OBJECT_GET_AI):
                context = dgi.read_uint32()
            # ... the second object notifies the parent it has changed location, which we ignore...
            elif dgi.matches_header([doid1], 5, STATESERVER_OBJECT_CHANGING_LOCATION):
                continue
            else:
                self.fail("Received unexpected message header.")

        # ... and we don't care about the reply from the parent (tested elsewhere)
        obj2.flush()

        # Then the second object should announce its presence to AI2...
        dg = Datagram.create([ai2chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.assertTrue(*ai2.expect(dg)) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        self.assertTrue(children2.expect_none())



        ### Test for SetAI on an object with children ### (continues from previous)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan) # AI channel
        conn.send(dg)
        obj1.flush()

        ai1expected = []
        ai2expected = []
        # Obj1 should tell its old AI channel and its children that it is changing AI...
        dg = Datagram.create([ai2chan, PARENT_PREFIX|doid1], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid1) # Id
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint64(ai2chan) # Old AI
        self.assertTrue(*children1.expect(dg))
        ai2expected.append(dg)
        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai1expected.append(dg)

        # Obj2 will also tell the old AI channel that it is changing AI...
        dg = Datagram.create([ai2chan], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(doid2) # Id
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint64(ai2chan) # Old AI
        self.assertTrue(children2.expect_none()) # It has no children
        ai2expected.append(dg)
        # ... and the new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        ai1expected.append(dg)

        self.assertTrue(*ai1.expect_multi(ai1expected, only=True))
        self.assertTrue(*ai2.expect_multi(ai2expected, only=True))



        ### Test for AI messages with various corner cases ### (continues from previous)
        # Now let's test the verification of the AI channel notification system.

        # A notification with an incorrect parent should do nothing:
        dg = Datagram.create([doid2], 0xABABABAB, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(0xABABABAB)
        dg.add_uint64(0x17FEEF71) # New AI
        dg.add_uint64(ai1chan) # Old AI
        conn.send(dg)
        self.assertTrue(*obj2.expect(dg)) # Ignore received dg
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
        self.assertTrue(*obj2.expect(dg)) # Ignore received dg
        self.assertTrue(obj2.expect_none())
        self.assertTrue(ai1.expect_none())
        self.assertTrue(obj1.expect_none())
        self.assertTrue(children2.expect_none())

        ### Test for EnterAIWithRequiredOther ### (continues from previous)
        # Delete the object for easy reuse
        deleteObject(conn, 5, doid2)
        children2.flush() # Ignore child propogation (there shouldn't be any)
        obj1.flush() # Ignore parent notification
        obj2.flush() # Ignore received delete
        ai2.flush() # Ignore AI notification
        ai1.flush() # Ignore AI notification

        # Recreate the second object with an optional field
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject1)
        dg.add_uint32(5773) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("I should've been asleep 5 hours ago!")
        conn.send(dg)

        # Set the AI of the object
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan)
        conn.send(dg)
        obj2.flush()

        # Expect an EnterAIWithRequiredOther instead of EnterAIWithRequired
        dg = Datagram.create([ai1chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject1)
        dg.add_uint32(5773) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("I should've been asleep 5 hours ago!")
        self.assertTrue(*ai1.expect(dg)) # Obj2 enters AI1 w/ Required Other



        ### Cleanup ###
        for doid in [doid1, doid2]:
            deleteObject(conn, 5, doid)
        for mdconn in [conn, ai1, ai2, obj1, obj2, children1, children2]:
            self.disconnect(mdconn)

    # Tests stateserver handling of the 'ram' keyword
    def test_ram(self):
        self.flush_failed()
        ai = self.connect(13000<<32|6800)
        ai.add_channel(13000<<32|4800)

        ### Test that ram fields are remembered ###
        # Create an object...
        createEmptyDTO1(ai, 5, 102000000, 13000, 6800, 12341234)

        # See if it shows up...
        dg = Datagram.create([13000<<32|6800], 102000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 102000000, 13000, 6800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        self.assertTrue(*ai.expect(dg))

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
        dg = Datagram.create([13000<<32|4800], 102000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, 102000000, 13000, 4800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        dg.add_uint16(1) # Other fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.assertTrue(*ai.expect(dg))


        ### Cleanup ###
        deleteObject(ai, 5, 102000000)
        self.disconnect(ai)

    # Tests the messages SET_LOCATION, CHANGING_LOCATION, and ENTER_LOCATION
    def test_set_location(self):
        self.flush_failed()
        conn = self.connect(5)
        conn.add_channel(0)

        doid0 = 14000
        doid1 = 105000000
        doid2 = 105050505
        obj0 = self.connect(doid0)
        obj1 = self.connect(doid1)
        obj2 = self.connect(doid2)
        location0 = self.connect(doid0<<32|9800)
        location2 = self.connect(doid2<<32|9900)

        ### Test for a call to SetLocation on object without previous location ###
        # Create an object...
        createEmptyDTO1(conn, 5, doid1, 0, 0, 43214321)

        # It shouldn't broadcast a location change because it doesn't have one.
        self.assertTrue(conn.expect_none())

        # Set the object's location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj1.flush()

        # Ignore changing_location and get_ai on parent
        obj0.flush()

        # See if it shows up...
        dg = Datagram.create([doid0<<32|9800], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, doid0, 9800, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        self.assertTrue(*location0.expect(dg))



        ### Test for a call to SetLocation on an object with existing location
        ### (continues from previous)
        # Create an object...
        createEmptyDTO1(conn, 5, doid2, 0, 0, 66668888)

        # It shouldn't broadcast a location change because it doesn't have one.
        self.assertTrue(conn.expect_none())

        # Move the first object into a zone of the second
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid2, zone=9900)
        conn.send(dg)
        obj1.flush()

        # See if it announces its departure from 14000<<32|9800...
        dg = Datagram.create([doid0<<32|9800, doid0, doid2], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(doid1) # ID
        appendMeta(dg, parent=doid2, zone=9900) # New location
        appendMeta(dg, parent=doid0, zone=9800) # Old location
        self.assertTrue(*location0.expect(dg))
        self.assertTrue(*obj0.expect(dg))
        self.assertTrue(*obj2.expect_multi([dg], only=False)) # Expect changing_location, ignore get_ai
        obj2.flush()

        # ...and its entry into the new location
        dg = Datagram.create([doid2<<32|9900], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, doid2, 9900, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        self.assertTrue(*location2.expect(dg))



        ### Test for non-propogation of SetLocation ### (continues from previous)
        # Move the parent object (#2) to a new zone
        conn.add_channel(PARENT_PREFIX|doid2)
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj2.flush()

        # Expect no messages to children
        self.assertTrue(conn.expect_none())
        conn.remove_channel(PARENT_PREFIX|doid2)

        # Expect only the second to change zones.
        dg = Datagram.create([doid0<<32|9800], doid2,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid2, doid0, 9800, DistributedTestObject1)
        dg.add_uint32(66668888) # setRequired1
        self.assertTrue(*location0.expect(dg))
        self.assertTrue(location0.expect_none())
        self.assertTrue(location2.expect_none())



        ### Test for SetLocation with an AI ### (continues from previous)
        # Give the first object an AI channel...
        conn.add_channel(225)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(225)
        conn.send(dg)
        obj1.flush()

        # ... and ignore any messages from that.
        conn.flush()

        # Change object location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE)
        conn.send(dg)
        obj1.flush()

        # Expect a ChangingLocation on the AI channel
        dg = Datagram.create([225, doid2, doid2<<32|9900], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(doid1)
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # New parent
        appendMeta(dg, parent=doid2, zone=9900) # Old parent
        self.assertTrue(*conn.expect(dg))

        # Ignore already tested behavior
        conn.remove_channel(225)
        location0.flush()
        obj0.flush()

        # Remove AI
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(0)
        conn.send(dg)
        conn.flush()
        obj1.flush()
        location2.flush()



        ### Test for SetLocation with an Owner ### (continues from previous)
        # Give the first object an owner...
        conn.add_channel(230)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(230)
        conn.send(dg)
        obj1.flush()

        # ... and ignore any messages from that.
        conn.flush()

        # Change object location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj1.flush()

        # Expect a ChangingLocation on the Owner channel
        dg = Datagram.create([230, doid0], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(doid1)
        appendMeta(dg, parent=doid0, zone=9800) # New parent
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # Old parent
        self.assertTrue(*conn.expect(dg))

        # Ignore already tested behavior
        conn.remove_channel(230)
        location0.flush()
        obj0.flush()

        # Remove Owner
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(0)
        conn.send(dg)
        conn.flush()
        obj1.flush()



        ### Test for SetLocation with a ram-broadcast field ### (continues from previous)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(doid1)
        dg.add_uint16(setBR1)
        dg.add_string("The Cutest Thing Ever!")
        conn.send(dg)
        obj1.flush()

        # Ignore field broadcast, tested in test_broadcast, not here...
        location0.flush()

        # Change object location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid2, zone=9900)
        conn.send(dg)
        obj1.flush()

        # Ignore changing location messages, already tested...
        obj0.flush()
        obj2.flush()
        location0.flush()

        # Should receive EnterLocationWithRequiredOther
        dg = Datagram.create([doid2<<32|9900], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, doid2, 9900, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("The Cutest Thing Ever!")
        self.assertTrue(*location2.expect(dg))



        ### Test for SetLocation with a non-broadcast ram field ### (continues from previous)
        # Delete the first object for reuse
        deleteObject(conn, 5, doid1)
        obj1.flush() # Ignore received message
        obj2.flush() # Ignore parent notification
        location2.flush() # Ignore location notification

        # Recreate the first object with a ram, non-broadcast field.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject3)
        dg.add_uint32(44004400) # setRequired1
        dg.add_uint32(88008800) # setRDB3
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setDb3)
        dg.add_string("Danger is my middle name!") # This field should be saved, but not broadcast
        conn.send(dg)

        # Change object location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj1.flush()

        # Ignore changing location messages, already tested...
        obj0.flush()

        # Should receive EnterLocationWithRequired|Other (non-broadcast fields are not included)
        dg = Datagram.create([doid0<<32|9800], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, doid0, 9800, DistributedTestObject3)
        dg.add_uint32(44004400) # setRequired1
        dg.add_uint32(88008800) # setRDB3
        dg.add_uint16(0) # Optional fields: 0
        self.assertTrue(*location0.expect(dg))


        ### Clean up ###
        for doid in [doid1, doid2]:
            deleteObject(conn, 5, doid)
        for mdconn in [conn, obj0, obj1, obj2, location0, location2]:
            self.disconnect(mdconn)


    # Tests stateserver handling of DistributedClassInheritance
    def test_inheritance(self):
        self.flush_failed()
        conn = self.connect(67000<<32|2000)

        ### Test for CreateObject on a subclass ###
        # Create a DTO3, which inherits from DTO1.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 110000000, 67000, 2000, DistributedTestObject3)
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        conn.send(dg)

        # Does it show up right?
        dg = Datagram.create([67000<<32|2000], 110000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 110000000, 67000, 2000, DistributedTestObject3)
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        self.assertTrue(*conn.expect(dg))

        # Cool, now let's test the broadcast messages:
        for field in [setRDB3, setRequired1]:
            dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_uint32(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            conn.send(dg)
            dg = Datagram.create([67000<<32|2000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_uint32(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            self.assertTrue(*conn.expect(dg))

        # This message is NOT part of DTO1/3 and should fail.
        # Bonus points for logging an ERROR log.
        dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(110000000)
        dg.add_uint16(setB2)
        dg.add_uint32(0x11225533)
        conn.send(dg)
        self.assertTrue(conn.expect_none())

        ### Cleanup ###
        deleteObject(conn, 5, 110000000)
        self.disconnect(conn)

    # Tests handling of various erroneous/bad/invalid messages
    def test_error(self):
        self.flush_failed()
        conn = self.connect(5)
        conn.add_channel(80000<<32|1234)

        ### Test for updating an invalid field ###
        # Create a regular object, this is not an error...
        createEmptyDTO1(conn, 5, 234000000, 80000, 1234, 819442)

        # The object should announce its entry to its location...
        dg = Datagram.create([80000<<32|1234], 234000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 234000000, 80000, 1234, DistributedTestObject1)
        dg.add_uint32(819442) # setRequired1
        self.assertTrue(*conn.expect(dg))

        # Send it an update on a bad field...
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(234000000)
        dg.add_uint16(0x1337)
        dg.add_uint32(0)
        conn.send(dg)

        # Nothing should happen and the SS should log an error.
        self.assertTrue(conn.expect_none())


        ### Test for updating with a truncated field ### (continues from previous)
        # How about a truncated update on a valid field?
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(234000000)
        dg.add_uint16(setRequired1)
        dg.add_uint16(0) # Whoops, 16-bit instead of 32-bit!
        conn.send(dg)

        # Nothing should happen + error.
        self.assertTrue(conn.expect_none())


        ### Test for creating an object with an in-use doid ### (continues from previous)
        # Let's try creating it again.
        createEmptyDTO1(conn, 5, 234000000, 80000, 1234, 1234567)

        # NOTHING SHOULD HAPPEN - additionally, the SS should log either an
        # error or a warning
        self.assertTrue(conn.expect_none())

        # Sanity check, make sure neither of the last three tests have changed any values
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF337) # Context
        dg.add_uint32(234000000) # Id
        conn.send(dg)

        # Values should be unchanged
        dg = Datagram.create([5], 234000000, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0xF337) # Context
        appendMeta(dg, 234000000, 80000, 1234, DistributedTestObject1)
        dg.add_uint32(819442) # setRequired1
        dg.add_uint16(0) # Optional fields: 0
        self.assertTrue(*conn.expect(dg))


        ### Test for creating an object without one if its required fields ###
        # Let's try making another one, but we'll forget the setRequired1.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 235000000, 80000, 1234, DistributedTestObject1)
        conn.send(dg)

        # Once again, nothing should happen and the SS should log an ERROR.
        self.assertTrue(conn.expect_none())

        # Additionally, no object should be received on a get all
        dg = Datagram.create([235000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF448) # Context
        dg.add_uint32(235000000) # Id
        conn.send(dg)

        # Values should be unchanged
        self.assertTrue(conn.expect_none())


        ### Test for creating an object with an unrecognized DistributedClass ###
        # Let's try making a bad object.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 236000000, 80000, 1234, 0x1337)
        conn.send(dg)

        # Nothing should happen and the SS should log an ERROR.
        self.assertTrue(conn.expect_none())

        # Additionally, no object should be received on a get all
        dg = Datagram.create([236000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF559) # Context
        dg.add_uint32(236000000) # Id
        conn.send(dg)

        # Values should be unchanged
        self.assertTrue(conn.expect_none())

        ### Cleanup ###
        deleteObject(conn, 5, 234000000)
        self.disconnect(conn)

    # Tests the message CREATE_OBJECT_WITH_REQUIRED_OTHER
    def test_create_with_other(self):
        self.flush_failed()
        conn = self.connect(90000<<32|4321)

        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545630000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        conn.send(dg)

        # We should get an ENTERZONE_WITH_REQUIRED_OTHER...
        dg = Datagram.create([90000<<32|4321], 545630000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545630000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        self.assertTrue(*conn.expect(dg))

        # If we try to include a non-ram as OTHER...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545640000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setB1)
        dg.add_uint8(42)
        conn.send(dg)

        # ...the object should show up, but without the non-RAM field.
        # Additionally, an ERROR should be logged.
        dg = Datagram.create([90000<<32|4321], 545640000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 545640000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        self.assertTrue(*conn.expect(dg))


        ### Cleanup ###
        deleteObject(conn, 5, 545630000)
        deleteObject(conn, 5, 545640000)
        self.disconnect(conn)

    # Tests for message GET_LOCATION
    def test_get_location(self):
        self.flush_failed()
        conn = self.connect(0x4a03)

        # Throw a few objects out there...
        createEmptyDTO1(conn, 5, 583312, 48000, 624800)
        createEmptyDTO1(conn, 5, 583311, 223315, 61444444)
        createEmptyDTO1(conn, 5, 583310, 51792, 5858182)

        # Now let's go on a treasure hunt:
        for doid, parent, zone in [
            (583312, 48000, 624800),
            (583311, 223315, 61444444),
            (583310, 51792, 5858182)]:

            # Try getting the location for each object, with some miscellaneous context
            context = (doid-500000)*2838
            dg = Datagram.create([doid], 0x4a03, STATESERVER_OBJECT_GET_LOCATION)
            dg.add_uint32(context)
            conn.send(dg)

            dg = Datagram.create([0x4a03], doid, STATESERVER_OBJECT_GET_LOCATION_RESP)
            dg.add_uint32(context)
            appendMeta(dg, doid, parent, zone)
            self.assertTrue(*conn.expect(dg))

            # Found the object! Now clean it up.
            deleteObject(conn, 5, doid)

        ### Cleanup ###
        self.disconnect(conn)

    # Tests for DELETE_AI_OBJECTS
    def test_delete_ai_objects(self):
        self.flush_failed()
        conn = self.connect(62222<<32|125)

        # Create an object...
        createEmptyDTO1(conn, 5, 201, 62222, 125, 6789)

        # ... with an AI channel...
        dg = Datagram.create([201], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(31337)
        conn.send(dg)

        # Ignore any noise...
        conn.flush()

        # Now let's try hitting the SS with a reset for the wrong AI:
        dg = Datagram.create([100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(41337)
        conn.send(dg)

        # Nothing should happen:
        self.assertTrue(conn.expect_none())

        # Now the correct AI:
        dg = Datagram.create([100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_uint64(31337)
        conn.send(dg)

        # Then object should die:
        dg = Datagram.create([62222<<32|125], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(201)
        self.assertTrue(*conn.expect(dg))

        ### Cleanup ###
        self.disconnect(conn)

    # Tests for messages GET_ALL, GET_FIELD, and GET_FIELDS
    def test_get(self):
        self.flush_failed()
        conn = self.connect(890)


        ### Test for GetAll on object with just required fields ###
        # Make an object
        createEmptyDTO1(conn, 5, 15000, 4, 2)

        # Get all from the object
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0x600DF00D)
        conn.send(dg)

        # Expect all data in response
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x600DF00D)
        appendMeta(dg, 15000, 4, 2, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # Optional fields: 0
        self.assertTrue(*conn.expect(dg))



        ### Test for GetField on a valid field with a set value ### (continues from previous)
        # Now get just a single field
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect get field response
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        self.assertTrue(*conn.expect(dg))

        ### Test for GetField on a valid field with no set value ### (continues from previous)
        # Get a field not present on the object...
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xBAD5EED) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(setBR1)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xBAD5EED) # Context
        dg.add_uint8(FAILURE)
        self.assertTrue(*conn.expect(dg))


        ### Test for GetField on an invalid field ### (continues from previous)
        # Get a field that is invalid for the object...
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0x5EE5BAD) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(0x1337) # Field id
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0x5EE5BAD) # Context
        dg.add_uint8(FAILURE)
        self.assertTrue(*conn.expect(dg))


        ### Test for GetFields on fields with set values ### (continues from previous)
        # First lets set a second field
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(15000) # ID
        dg.add_uint16(setBR1)
        dg.add_string("MY HAT IS AWESOME!!!")
        conn.send(dg)

        # Now let's try getting multiple fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x600D533D)
        dg.add_uint32(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint16(setBR1)
        conn.send(dg)

        # Expect both fields in response
        dg = conn.recv_maybe()
        self.assertTrue(dg is not None, "Did not receive a GetFieldsResp")
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP))
        self.assertEquals(dgi.read_uint32(), 0x600D533D) # Context
        self.assertEquals(dgi.read_uint8(), SUCCESS)
        self.assertEquals(dgi.read_uint16(), 2) # Field count
        hasRequired1 = False
        hasBR1 = False
        for x in xrange(2):
            field = dgi.read_uint16()
            if field is setRequired1:
                hasRequired1 = True
                self.assertEquals(dgi.read_uint32(), 0)
            elif field is setBR1:
                hasBR1 = True
                self.assertEquals(dgi.read_string(), "MY HAT IS AWESOME!!!")
            else:
                self.fail("Unexpected field type")
        self.assertTrue(hasRequired1 and hasBR1)


        ### Test for GetFields with mixed set and unset fields ### (continues from previous)
        # Send GetFields with mixed set and unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x711E644E) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setBR1)
        dg.add_uint16(setBRA1)
        conn.send(dg)

        # Expect only one field back
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x711E644E) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setBR1)
        dg.add_string("MY HAT IS AWESOME!!!")
        self.assertTrue(*conn.expect(dg))


        ### Test for GetFields with only unset fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x822F755F) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setBRA1)
        dg.add_uint16(setBRO1)
        conn.send(dg)

        # Expect success with no fields back
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x822F755F) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(0) # Field count
        self.assertTrue(*conn.expect(dg))


        ### Test for GetFields with only invalid fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x5FFC422C) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint16(setDb3)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x5FFC422C) # Context
        dg.add_uint8(FAILURE)
        self.assertTrue(*conn.expect(dg))


        ### Test for GetFields with mixed valid and invalid fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x4EEB311B) # Context
        dg.add_uint32(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x4EEB311B) # Context
        dg.add_uint8(FAILURE)
        self.assertTrue(*conn.expect(dg))


        ### Cleanup ###
        deleteObject(conn, 5, 15000)
        self.disconnect(conn)

    # Tests the message SET_FIELDS
    def test_set_fields(self):
        self.flush_failed()
        conn = self.connect(5985858)
        location = self.connect(12512<<32|66161)

        ### Test for SetFields with broadcast and ram filds ###
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 55555, 12512, 66161, DistributedTestObject3)
        dg.add_uint32(0) # setRequired1
        dg.add_uint32(0) # setRDB3
        conn.send(dg)

        # Ignore the entry message...
        location.flush()

        # Send a multi-field update with two broadcast fields and one ram.
        dg = Datagram.create([55555], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_uint32(55555) # ID
        dg.add_uint16(3) # 3 fields:
        dg.add_uint16(setDb3)
        dg.add_string('Time is candy')
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        conn.send(dg)

        # Verify that the broadcasts go out, in order... (these must be in the correct order)
        dg = Datagram.create([12512<<32|66161], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_uint32(55555) # ID
        dg.add_uint16(2) # 2 fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        # TODO: Implement partial field broadcasts, then uncomment the test
        #self.assertTrue(*location.expect(dg))
        location.flush()

        # Get the ram fields to make sure they're set
        dg = Datagram.create([55555], 5985858, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0x5111) # Context
        dg.add_uint32(55555) #ID
        conn.send(dg)

        # See if those updates have properly gone through...
        dg = Datagram.create([5985858], 55555, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x5111) # Context
        appendMeta(dg, 55555, 12512, 66161, DistributedTestObject3)
        dg.add_uint32(0xD00D) # setRequired1
        dg.add_uint32(0) # setRDB3
        dg.add_uint16(1) # 1 extra field:
        dg.add_uint16(setDb3)
        dg.add_string('Time is candy')
        self.assertTrue(*conn.expect(dg))

        ### Cleanup ###
        deleteObject(conn, 5985858, 55555)
        self.disconnect(location)
        self.disconnect(conn)

    # Tests stateserver handling of the 'ownrecv' keyword
    def test_ownrecv(self):
        self.flush_failed()
        conn = self.connect(14253648)

        ### Test for broadcast of empty
        # Create an object
        createEmptyDTO1(conn, 5, 77878788, 88833, 99922, 0)

        # Set the object's owner
        dg = Datagram.create([77878788], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(14253648)
        conn.send(dg)

        # Ignore EnterOwner message, not testing that here
        conn.flush()

        # Set field on an ownrecv message...
        dg = Datagram.create([77878788], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(77878788)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        conn.send(dg)

        # See if our owner channel gets it.
        dg = Datagram.create([14253648], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(77878788)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        self.assertTrue(*conn.expect(dg))

        # Delete the object...
        deleteObject(conn, 5, 77878788)

        # And expect to be notified
        dg = Datagram.create([14253648], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(77878788)
        self.assertTrue(*conn.expect(dg))

        ### Cleanup ###
        self.disconnect(conn)

    # Tests the message SET_OWNER, CHANGING_OWNER, ENTER_OWNER
    def test_set_owner(self):
        self.flush_failed()
        conn = self.connect(5)

        owner1chan = 14253647
        owner2chan = 22446622
        doid1 = 74635241
        doid2 = 0x4a0351

        owner1 = self.connect(owner1chan)
        owner2 = self.connect(owner2chan)

        ### Test for SetOwner on an object with no owner ###
        # Make an object to play around with
        createEmptyDTO1(conn, 5, doid1, required1=0)

        # Set the object's owner...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(owner1chan)
        conn.send(dg)

        # ... and see if it enters the owner.
        dg = Datagram.create([owner1chan], doid1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        self.assertTrue(*owner1.expect(dg))



        ### Test for SetOwner on an object with an existing owner ### (continues from previous)
        # Change owner to someone else...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(owner2chan)
        conn.send(dg)

        # ... expecting a changing owner ...
        dg = Datagram.create([owner1chan], 5, STATESERVER_OBJECT_CHANGING_OWNER)
        dg.add_uint32(doid1)
        dg.add_uint64(owner2chan) # New owner
        dg.add_uint64(owner1chan) # Old owner
        self.assertTrue(*owner1.expect(dg))
        # ... and see if it enters the new owner.
        dg = Datagram.create([owner2chan], doid1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        self.assertTrue(*owner2.expect(dg))



        ### Test for SetOwner on an object with owner fields and non-owner, non-broadcast fields ###
        # Create an object with a bunch of types of fields
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject3)
        dg.add_uint32(0) # setRequired1
        dg.add_uint32(0) # setRDB3
        dg.add_uint16(3) # Optional fields: 3
        dg.add_uint16(setDb3)
        dg.add_string("Hey Princess! Want to do the SCIENCE DANCE with me?")
        dg.add_uint16(setBRO1)
        dg.add_uint32(0x1337)
        dg.add_uint16(setBR1)
        dg.add_string("I feel radder, faster... more adequate!")
        conn.send(dg)

        # Set the object's owner...
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_uint64(owner1chan)
        conn.send(dg)

        # ... and see if it enters the owner with only broadcast and/or ownrecv fields.
        dg = owner1.recv_maybe()
        self.assertTrue(dg is not None, "Did not receive a datagram when expecting EnterOwner")
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([owner1chan], doid2,
                STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER))
        self.assertEquals(dgi.read_uint32(), doid2) # Id
        self.assertEquals(dgi.read_uint64(), 0) # Location (parent<<32|zone)
        self.assertEquals(dgi.read_uint16(), DistributedTestObject3)
        self.assertEquals(dgi.read_uint32(), 0) # setRequired1
        self.assertEquals(dgi.read_uint32(), 0) # setRDB3
        self.assertEquals(dgi.read_uint16(), 2) # Optional fields: 2
        hasBR1, hasBRO1 = False, False
        for x in xrange(2):
            field = dgi.read_uint16()
            if field is setBR1:
                hasBR1 = True
                self.assertEquals(dgi.read_string(), "I feel radder, faster... more adequate!")
            elif field is setBRO1:
                hasBRO1 = True
                self.assertEquals(dgi.read_uint32(), 0x1337)
            else:
                self.fail("Received unexpected field: " + str(field))
        self.assertTrue(hasBR1 and hasBRO1) # setBR1 and setBRO1 in ENTER_OWNER_WITH_REQUIRED_OTHER

        ### Cleanup ###
        deleteObject(conn, 5, doid1)
        deleteObject(conn, 5, doid2)
        self.disconnect(owner1)
        self.disconnect(owner2)
        self.disconnect(conn)

    # Tests stateserver handling of molecular fields
    def test_molecular(self):
        self.flush_failed()
        conn = self.connect(13371337)
        location0 = self.connect(88<<32|99)

        ### Test for broadcast of a molecular SetField is molecular ###
        # Create an object
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        conn.send(dg)

        # Verify its entry...
        dg = Datagram.create([88<<32|99], 73317331, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        self.assertTrue(*location0.expect(dg))

        # Send a molecular update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        conn.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<32|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        self.assertTrue(*location0.expect(dg))



        ### Test for molecular SetField properly updating the individual values
        ### (continues from previous)
        # Look at the object and see if the requireds are updated...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0) # Context
        dg.add_uint32(73317331)
        conn.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0) # Context
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(77) # setZ
        dg.add_uint16(0) # Optional fields: 0
        self.assertTrue(*conn.expect(dg))



        ### Test for molecular SetField with a ram, not-required, fields
        ### (continues from previous)
        # Now try a RAM update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        conn.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<32|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        self.assertTrue(*location0.expect(dg))

        # A query should have all of the individual fields, not the molecular.
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(1) # Context
        dg.add_uint32(73317331) # ID
        conn.send(dg)

        dg = conn.recv_maybe()
        self.assertTrue(dg is not None, "Did not receive datagram when expecting GetAllResp")
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP))
        self.assertEquals(dgi.read_uint32(), 1) # Context
        self.assertEquals(dgi.read_uint32(), 73317331) # Id
        self.assertEquals(dgi.read_uint32(), 88) # Parent
        self.assertEquals(dgi.read_uint32(), 99) # Zone
        self.assertEquals(dgi.read_uint16(), DistributedTestObject4)
        self.assertEquals(dgi.read_uint32(), 55) # SetX
        self.assertEquals(dgi.read_uint32(), 66) # SetY
        self.assertEquals(dgi.read_uint32(), 999999) # SetUnrelated
        self.assertEquals(dgi.read_uint32(), 77) # SetZ
        self.assertEquals(dgi.read_uint16(), 3) # Optional fields: 3
        hasOne, hasTwo, hasThree = False, False, False
        for x in xrange(3):
            field = dgi.read_uint16()
            if field is setOne:
                hasOne = True
                self.assertEquals(dgi.read_uint8(), 1)
            elif field is setTwo:
                hasTwo = True
                self.assertEquals(dgi.read_uint8(), 2)
            elif field is setThree:
                hasThree = True
                self.assertEquals(dgi.read_uint8(), 3)
        self.assertTrue(hasOne and hasTwo and hasThree)



        ### Test for GetFields on mixed molecular and atomic fields ### (continues from previous)
        # Now let's test querying individual fields...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_uint32(73317331) # ID
        dg.add_uint16(5) # Field count: 5
        dg.add_uint16(setXyz)
        dg.add_uint16(setOne)
        dg.add_uint16(setUnrelated)
        dg.add_uint16(set123)
        dg.add_uint16(setX)
        conn.send(dg)

        dg = conn.recv_maybe()
        self.assertTrue(dg is not None,  "Did not receive datagram when expecting GetAllResp")
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([13371337], 73317331, STATESERVER_OBJECT_GET_FIELDS_RESP))
        self.assertEquals(dgi.read_uint32(), 0xBAB55EED) # Context
        self.assertEquals(dgi.read_uint8(), SUCCESS)
        self.assertEquals(dgi.read_uint16(), 5) # Field count: 5
        for x in xrange(5):
            field = dgi.read_uint16()
            if field is setXyz:
                self.assertEquals(dgi.read_uint32(), 55)
                self.assertEquals(dgi.read_uint32(), 66)
                self.assertEquals(dgi.read_uint32(), 77)
            elif field is setOne:
                self.assertEquals(dgi.read_uint8(), 1)
            elif field is setUnrelated:
                self.assertEquals(dgi.read_uint32(), 999999)
            elif field is set123:
                self.assertEquals(dgi.read_uint8(), 1)
                self.assertEquals(dgi.read_uint8(), 2)
                self.assertEquals(dgi.read_uint8(), 3)
            elif field is setX:
                self.assertEquals(dgi.read_uint32(), 55)
            else:
                self.fail("Received unexpected field ("+str(field)+") in GET_FIELDS_RESP.")


        ### Cleanup ###
        deleteObject(conn, 5, 73317331)
        self.disconnect(location0)
        self.disconnect(conn)

    # Tests the message GET_ZONES_OBJECTS
    def test_get_zones_objects(self):
        self.flush_failed()
        conn = self.connect(5)

        doid0 = 1000 # Root object
        doid1 = 2001
        doid2 = 2002
        doid3 = 2003
        doid4 = 2004
        doid5 = 2005
        doid6 = 3006 # Child of child

        ### Test for GetZonesObjects ###
        # Make a bunch of objects
        createEmptyDTO1(conn, 5, doid0)
        createEmptyDTO1(conn, 5, doid1, doid0, 912)
        createEmptyDTO1(conn, 5, doid2, doid0, 912)
        createEmptyDTO1(conn, 5, doid3, doid0, 930)
        createEmptyDTO1(conn, 5, doid4, doid0, 940)
        createEmptyDTO1(conn, 5, doid5, doid0, 950)
        createEmptyDTO1(conn, 5, doid6, doid1, 860)

        # Ask for objects from some of the zones...
        dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_GET_ZONES_OBJECTS)
        dg.add_uint32(0xF337) # Context
        dg.add_uint32(doid0) # Parent Id
        dg.add_uint16(2) # Zone count
        dg.add_uint32(912)
        dg.add_uint32(930)
        conn.send(dg)

        expected = []
        # ... expecting the count of objects in the zone...
        dg = Datagram.create([5], doid0, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(0xF337) # Context
        dg.add_uint32(3) # Count of objects [(912, [obj1, obj2]), (930, [obj3])]
        expected.append(dg)
        # ... and object1's enter zone message...
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, doid0, 912, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        expected.append(dg)
        # ... and object2's enter zone message...
        dg = Datagram.create([5], doid2, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid2, doid0, 912, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        expected.append(dg)
        # ... and object3's enter zone message...
        dg = Datagram.create([5], doid3, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid3, doid0, 930, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        expected.append(dg)
        self.assertTrue(*conn.expect_multi(expected, only=True))

        # Shouldn't receive messages from any of the other objects
        self.assertTrue(conn.expect_none())


        ### Cleanup ###
        for doid in (doid0, doid1, doid2, doid3, doid4, doid5, doid6):
            deleteObject(conn, 5, doid)
        self.disconnect(conn)

    # Tests the OBJECT_DELETE_CHILDREN message and propogation of delete ram
    def test_delete_children(self):
        self.flush_failed()
        conn = self.connect(5)

        doid0 = 101100 # Root object
        doid1 = 202200
        doid2 = 203300

        children0 = self.connect(PARENT_PREFIX|doid0)
        location1 = self.connect(doid0<<32|710)
        location20 = self.connect(doid0<<32|720)
        location21 = self.connect(doid1<<32|720)

        ### Test for ObjectDeleteChildren ###
        # Create an object tree
        createEmptyDTO1(conn, 5, doid0)
        createEmptyDTO1(conn, 5, doid1, doid0, 710)
        createEmptyDTO1(conn, 5, doid2, doid0, 720)

        # Ignore entry broadcasts
        location1.flush()
        location20.flush()

        # Send delete children
        dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_uint32(doid0)
        conn.send(dg)

        # Expect children to receive delete children...
        dg = Datagram.create([PARENT_PREFIX|doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_uint32(doid0)
        self.assertTrue(*children0.expect(dg))
        # ... and then broadcast their own deletion
        dg = Datagram.create([doid0<<32|710], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(doid1)
        self.assertTrue(*location1.expect(dg))
        dg = Datagram.create([doid0<<32|720], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(doid2)
        self.assertTrue(*location20.expect(dg))



        ### Test for DeleteRam propogation to children ###
        # Add our children in multi-tier tree
        createEmptyDTO1(conn, 5, doid1, doid0, 710)
        createEmptyDTO1(conn, 5, doid2, doid1, 720)

        # Ignore entry broadcasts
        location1.flush()
        location21.flush()

        # Delete the root object
        dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(doid0)
        conn.send(dg)

        # Expect the first object to receive delete children from object zero...
        dg = Datagram.create([PARENT_PREFIX|doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_uint32(doid0)
        self.assertTrue(*children0.expect(dg))
        # ... and delete itself...
        dg = Datagram.create([doid0<<32|710], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(doid1)
        self.assertTrue(*location1.expect(dg))
        # ... and propogate the deletion to its child...
        dg = Datagram.create([doid1<<32|720], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(doid2)
        self.assertTrue(*location21.expect(dg))


        ### Cleanup ###
        for mdconn in [conn, children0, location1, location20, location21]:
            self.disconnect(mdconn)

    # Tests the keyword clrecv
    def test_clrecv(self):
        self.flush_failed()
        conn = self.connect(0xBAD << 32 | 0xF001)

        doid1 = 0xF00

        ### Test for clrecv keyword ###
        # Created a distributed chunk
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 0xBAD, 0xF001, DistributedChunk)
        dg.add_uint16(12) # blockList size in bytes (contains 1 element)
        dg.add_uint32(43) # blockList[0].x
        dg.add_uint32(54) # blockList[0].y
        dg.add_uint32(65) # blockList[0].z
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(lastBlock)
        dg.add_uint32(0) # lastBlock.x
        dg.add_uint32(0) # lastBlock.y
        dg.add_uint32(0) # lastBlock.z
        conn.send(dg)

        # Expect entry message
        dg = Datagram.create([0xBAD << 32 | 0xF001], doid1,
            STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 0xBAD, 0xF001, DistributedChunk)
        dg.add_uint16(12) # blockList size in bytes (contains 1 element)
        dg.add_uint32(43) # blockList[0].x
        dg.add_uint32(54) # blockList[0].y
        dg.add_uint32(65) # blockList[0].z
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(lastBlock)
        dg.add_uint32(0) # lastBlock.x
        dg.add_uint32(0) # lastBlock.y
        dg.add_uint32(0) # lastBlock.z
        self.assertTrue(*conn.expect(dg))

        # Update the object with some new values
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_uint32(doid1) # Id
        dg.add_uint16(3) # Num fields: 3
        dg.add_uint16(blockList)
        dg.add_uint16(24) # blockList size in bytes (contains 2 elements)
        dg.add_uint32(43) # blockList[0].x
        dg.add_uint32(54) # blockList[0].y
        dg.add_uint32(65) # blockList[0].z
        dg.add_uint32(171) # blockList[1].x
        dg.add_uint32(282) # blockList[1].y
        dg.add_uint32(393) # blockList[1].z
        dg.add_uint16(lastBlock)
        dg.add_uint32(43) # lastBlock.x
        dg.add_uint32(54) # lastBlock.y
        dg.add_uint32(65) # lastBlock.z
        dg.add_uint16(newBlock)
        dg.add_uint32(171) # lastBlock.x
        dg.add_uint32(282) # lastBlock.y
        dg.add_uint32(393) # lastBlock.z
        conn.send(dg)

        # Expect only the broadcast field to be sent to the location
        dg = Datagram.create([0xBAD << 32 | 0xF001], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(doid1) # Id
        dg.add_uint16(newBlock)
        dg.add_uint32(171) # lastBlock.x
        dg.add_uint32(282) # lastBlock.y
        dg.add_uint32(393) # lastBlock.z
        self.assertTrue(*conn.expect(dg))


        ### Cleanup ###
        deleteObject(conn, 5, doid1)
        self.disconnect(conn)

if __name__ == '__main__':
    unittest.main()
