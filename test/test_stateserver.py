#!/usr/bin/env python2
import unittest, time
from common.unittests import ProtocolTest
from common.astron import *
from common.dcfile import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    threaded: %s

general:
    dc_files:
        - %r

roles:
    - type: stateserver
      control: 100100
""" % (USE_THREADING, test_dc)

def appendMeta(datagram, doid=None, parent=None, zone=None, dclass=None):
    if doid is not None:
        datagram.add_doid(doid)
    if parent is not None:
        datagram.add_doid(parent)
    if zone is not None:
        datagram.add_zone(zone)
    if dclass is not None:
        datagram.add_uint16(dclass)

def createEmptyDTO1(conn, sender, doid, parent=0, zone=0, required1=0):
    dg = Datagram.create([100100], sender, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
    appendMeta(dg, doid, parent, zone, DistributedTestObject1)
    dg.add_uint32(required1)
    conn.send(dg)

def deleteObject(conn, sender, doid):
    dg = Datagram.create([doid], sender, STATESERVER_OBJECT_DELETE_RAM)
    dg.add_doid(doid)
    conn.send(dg)

CONN_POOL_SIZE = 8
class TestStateServer(ProtocolTest):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        cls.conn_pool = []
        cls.conn_used = []
        for x in xrange(CONN_POOL_SIZE):
            cls.conn_pool.append(ChannelConnection('127.0.0.1', 57123))

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
        ai = self.connect(5000<<ZONE_SIZE_BITS|1500)
        parent = self.connect(5000)
        children = self.connect(PARENT_PREFIX|101000000)

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
                if dgi.matches_header([5000], 5, STATESERVER_OBJECT_CHANGING_LOCATION)[0]:
                    received = True
                    self.assertEquals(dgi.read_doid(), 101000000) # Id
                    self.assertEquals(dgi.read_doid(), 5000) # New parent
                    self.assertEquals(dgi.read_zone(), 1500) # New zone
                    self.assertEquals(dgi.read_doid(), INVALID_DO_ID) # Old parent
                    self.assertEquals(dgi.read_zone(), INVALID_ZONE) # Old zone
                # .. and ask it for its AI, which we're not testing here and can ignore
                elif dgi.matches_header([5000], 101000000, STATESERVER_OBJECT_GET_AI)[0]:
                    continue
                else:
                    failA = dgi.matches_header([5000], 5, STATESERVER_OBJECT_CHANGING_LOCATION)[1]
                    failB = dgi.matches_header([5000], 101000000, STATESERVER_OBJECT_GET_AI)[1]
                    self.fail("Received message-type(%d), expected:"
                              "\n\tObjectChangingLocation -- %s\n\tObjectGetAI -- %s"
                              % (msgtype, failA, failB))
            self.assertTrue(received) # Parent received ChangingLocation

            # The object should announce its entry to the zone-channel...
            dg = Datagram.create([5000<<ZONE_SIZE_BITS|1500], 101000000,
                                 STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
            appendMeta(dg, 101000000, 5000, 1500, DistributedTestObject1)
            dg.add_uint32(6789) # setRequired1
            self.expect(ai, dg)

            # .. as well as waking its children and getting their location.
            dg = Datagram.create([PARENT_PREFIX|101000000], 101000000,
                                 STATESERVER_OBJECT_GET_LOCATION)
            dg.add_uint32(STATESERVER_CONTEXT_WAKE_CHILDREN)
            self.expect(children, dg)

            ### Test for DeleteRam ### (continues from previous)
            # Destroy our object...
            deleteObject(ai, 5, 101000000)

            # Object should tell its parent it is going away...
            dg = Datagram.create([5000], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
            dg.add_doid(101000000)
            appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # New location
            appendMeta(dg, parent=5000, zone=1500) # Old location
            self.expect(parent, dg)

            # Object should announce its disappearance...
            dg = Datagram.create([5000<<ZONE_SIZE_BITS|1500], 5, STATESERVER_OBJECT_DELETE_RAM)
            dg.add_doid(101000000)
            self.expect(ai, dg)

        # We're done here...
        ### Cleanup ###
        self.disconnect(parent)
        self.disconnect(ai)

    # Tests the handling of the broadcast keyword by the stateserver
    def test_broadcast(self):
        self.flush_failed()
        ai = self.connect(5000<<ZONE_SIZE_BITS|1500)

        ### Test for Broadcast to location ###
        # Create a DistributedTestObject2...
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 101000005, 5000, 1500, DistributedTestObject2)
        ai.send(dg)

        # Ignore the entry message, we aren't testing that here.
        ai.flush()

        # Hit it with an update on setB2.
        dg = Datagram.create([101000005], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        ai.send(dg)

        # Object should broadcast that update
        # Note: Sender should be original sender (in this case 5). This is so AIs
        #       can see who the update ultimately comes from for e.g. an airecv/clsend.
        dg = Datagram.create([5000<<ZONE_SIZE_BITS|1500], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        self.expect(ai, dg)

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
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 100010, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        conn.send(dg)

        dg = Datagram.create([100010], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(1300)
        conn.send(dg)

        # Ignore EnterAI message, not testing that here
        conn.flush()

        dg = Datagram.create([100010], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(100010)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        conn.send(dg)

        # Now the AI should see it...
        # Note: The ai should be a separate recipient of the same message sent
        #       to the objects location channel.
        dg = Datagram.create([1300, (5000<<ZONE_SIZE_BITS|1500)], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(100010)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.expect(conn, dg)


        ### Test for AI notification of object deletions ### (continues from previous)
        # Delete the object
        deleteObject(conn, 5, 100010)

        # See if the AI receives the delete.
        dg = Datagram.create([1300, (5000<<ZONE_SIZE_BITS|1500)], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(100010)
        self.expect(conn, dg)

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
        self.expectNone(conn)

        # We'll ignore the wake children messages
        children1.flush()

        # First object belongs to AI1...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(ai1chan) # AI channel
        conn.send(dg)
        obj1.flush()

        # Obj1 should announce its presence to AI1...
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.expect(ai1, dg) # AI received ENTER_AI

        # ... but should not tell its children (it has none) ...
        self.expectNone(children1)



        ### Test for SetAI on an object with an existing AI ### (continues from previous)
        # Set AI to new AI channel
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(ai2chan) # AI channel
        conn.send(dg)
        obj1.flush()

        # Obj1 should tell its old AI channel that it is changing AI...
        dg = Datagram.create([ai1chan], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_doid(doid1) # Id
        dg.add_channel(ai2chan) # New AI
        dg.add_channel(ai1chan) # Old AI
        self.expect(ai1, dg)

        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai2chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.expect(ai2, dg)



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
            if dgi.matches_header([doid1], doid2, STATESERVER_OBJECT_GET_AI)[0]:
                context = dgi.read_uint32()
            # ... the second object notifies the parent it has changed location, which we ignore...
            elif dgi.matches_header([doid1], 5, STATESERVER_OBJECT_CHANGING_LOCATION)[0]:
                continue
            else:
                self.fail("Received unexpected or non-matching header.")

        # ... and the parent should reply with its AI server.
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_doid(doid1)
        dg.add_channel(ai2chan)
        self.expect(obj2, dg) # Receiving GET_AI_RESP from parent

        # Check to make sure we're receiving a wake children GetLocation message, and not something else
        dg = Datagram.create([PARENT_PREFIX|doid2], doid2, STATESERVER_OBJECT_GET_LOCATION)
        dg.add_uint32(STATESERVER_CONTEXT_WAKE_CHILDREN)
        self.expect(children2, dg)

        # Then the second object should announce its presence to AI2...
        dg = Datagram.create([ai2chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.expect(ai2, dg) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        self.expectNone(children2)



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
        self.expect(ai2, dg) # Obj2 enters AI2

        # ... but should not tell its children because it has none.
        self.expectNone(children2)



        ### Test for SetAI on an object with children ### (continues from previous)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(ai1chan) # AI channel
        conn.send(dg)
        obj1.flush()

        ai1expected = []
        ai2expected = []
        # Obj1 should tell its old AI channel and its children that it is changing AI...
        dg = Datagram.create([ai2chan, PARENT_PREFIX|doid1], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_doid(doid1) # Id
        dg.add_channel(ai1chan) # New AI
        dg.add_channel(ai2chan) # Old AI
        self.expect(children1, dg)
        ai2expected.append(dg)
        # ... and its new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid1, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        ai1expected.append(dg)

        # Obj2 will also tell the old AI channel that it is changing AI...
        dg = Datagram.create([ai2chan], 5, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_doid(doid2) # Id
        dg.add_channel(ai1chan) # New AI
        dg.add_channel(ai2chan) # Old AI
        self.expectNone(children2) # It has no children
        ai2expected.append(dg)
        # ... and the new AI channel that it is entering.
        dg = Datagram.create([ai1chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, doid2, doid1, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        ai1expected.append(dg)

        self.expectMany(ai1, ai1expected)
        self.expectMany(ai2, ai2expected)



        ### Test for AI messages with various corner cases ### (continues from previous)
        # Now let's test the verification of the AI channel notification system.

        # A notification with an incorrect parent should do nothing:
        dg = Datagram.create([doid2], 0xABABABAB, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_doid(0xABABABAB)
        dg.add_channel(0x17FEEF71) # New AI
        dg.add_channel(ai1chan) # Old AI
        conn.send(dg)
        self.expect(obj2, dg) # Ignore received dg
        self.expectNone(obj2)
        self.expectNone(ai1)
        self.expectNone(obj1)
        self.expectNone(children2)

        # A notification with the same AI channel should also do nothing:
        dg = Datagram.create([doid2], doid1, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_doid(doid1)
        dg.add_channel(ai1chan)
        dg.add_channel(ai1chan)
        obj1.send(dg)
        self.expect(obj2, dg) # Ignore received dg
        self.expectNone(obj2)
        self.expectNone(ai1)
        self.expectNone(obj1)
        self.expectNone(children2)

        ### Test for EnterAIWithRequiredOther ### (continues from previous)
        # Delete the object for easy reuse
        deleteObject(conn, 5, doid2)
        children2.flush() # Ignore child propogation (there shouldn't be any)
        obj1.flush() # Ignore parent notification
        obj2.flush() # Ignore received delete
        ai2.flush() # Ignore AI notification
        ai1.flush() # Ignore AI notification

        # Recreate the second object with an optional field
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject1)
        dg.add_uint32(5773) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("I should've been asleep 5 hours ago!")
        conn.send(dg)

        # Set the AI of the object
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(ai1chan)
        conn.send(dg)
        obj2.flush()

        # Expect an EnterAIWithRequiredOther instead of EnterAIWithRequired
        dg = Datagram.create([ai1chan], doid2, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 0, 0, DistributedTestObject1)
        dg.add_uint32(5773) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("I should've been asleep 5 hours ago!")
        self.expect(ai1, dg) # Obj2 enters AI1 w/ Required Other



        ### Cleanup ###
        for doid in [doid1, doid2]:
            deleteObject(conn, 5, doid)
        for mdconn in [conn, ai1, ai2, obj1, obj2, children1, children2]:
            self.disconnect(mdconn)

    # Tests stateserver handling of the 'ram' keyword
    def test_ram(self):
        self.flush_failed()
        ai = self.connect(13000<<ZONE_SIZE_BITS|6800)
        ai.add_channel(13000<<ZONE_SIZE_BITS|4800)

        ### Test that ram fields are remembered ###
        # Create an object...
        createEmptyDTO1(ai, 5, 102000000, 13000, 6800, 12341234)

        # See if it shows up...
        dg = Datagram.create([13000<<ZONE_SIZE_BITS|6800], 102000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 102000000, 13000, 6800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        self.expect(ai, dg)

        # At this point we don't care about zone 6800.
        ai.remove_channel(13000<<ZONE_SIZE_BITS|6800)

        # Hit it with a RAM update.
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(102000000)
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
        dg = Datagram.create([13000<<ZONE_SIZE_BITS|4800], 102000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, 102000000, 13000, 4800, DistributedTestObject1)
        dg.add_uint32(12341234) # setRequired1
        dg.add_uint16(1) # Other fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.expect(ai, dg)


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
        location0 = self.connect(doid0<<ZONE_SIZE_BITS|9800)
        location2 = self.connect(doid2<<ZONE_SIZE_BITS|9900)

        ### Test for a call to SetLocation on object without previous location ###
        # Create an object...
        createEmptyDTO1(conn, 5, doid1, 0, 0, 43214321)

        # It shouldn't broadcast a location change because it doesn't have one.
        self.expectNone(conn)

        # Set the object's location
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj1.flush()

        # Ignore changing_location and get_ai on parent
        obj0.flush()

        # See if it shows up...
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|9800], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, doid0, 9800, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        self.expect(location0, dg)



        ### Test for a call to SetLocation on an object with existing location
        ### (continues from previous)
        # Create an object...
        createEmptyDTO1(conn, 5, doid2, 0, 0, 66668888)

        # It shouldn't broadcast a location change because it doesn't have one.
        self.expectNone(conn)

        # Move the first object into a zone of the second
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid2, zone=9900)
        conn.send(dg)
        obj1.flush()

        # See if it announces its departure from 14000<<ZONE_SIZE_BITS|9800...
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|9800, doid0, doid2], 5,
                             STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(doid1) # ID
        appendMeta(dg, parent=doid2, zone=9900) # New location
        appendMeta(dg, parent=doid0, zone=9800) # Old location
        self.expect(location0, dg)
        self.expect(obj0, dg)
        self.expectMany(obj2, [dg], ignoreExtra = True) # Expect changing_location, ignore get_ai
        obj2.flush()

        # ...and its entry into the new location
        dg = Datagram.create([doid2<<ZONE_SIZE_BITS|9900], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, doid2, 9900, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        self.expect(location2, dg)



        ### Test for non-propogation of SetLocation ### (continues from previous)
        # Move the parent object (#2) to a new zone
        conn.add_channel(PARENT_PREFIX|doid2)
        dg = Datagram.create([doid2], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=9800)
        conn.send(dg)
        obj2.flush()

        # Expect no messages to children
        self.expectNone(conn)
        conn.remove_channel(PARENT_PREFIX|doid2)

        # Expect only the second to change zones.
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|9800], doid2,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid2, doid0, 9800, DistributedTestObject1)
        dg.add_uint32(66668888) # setRequired1
        self.expect(location0, dg)
        self.expectNone(location0)
        self.expectNone(location2)



        ### Test for SetLocation with an AI ### (continues from previous)
        # Give the first object an AI channel...
        conn.add_channel(225)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(225)
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
        dg = Datagram.create([225, doid2, doid2<<ZONE_SIZE_BITS|9900], 5,
                             STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(doid1)
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # New parent
        appendMeta(dg, parent=doid2, zone=9900) # Old parent
        self.expect(conn, dg)

        # Ignore already tested behavior
        conn.remove_channel(225)
        location0.flush()
        obj0.flush()

        # Remove AI
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(0)
        conn.send(dg)
        conn.flush()
        obj1.flush()
        location2.flush()



        ### Test for SetLocation with an Owner ### (continues from previous)
        # Give the first object an owner...
        conn.add_channel(230)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_channel(230)
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
        dg.add_doid(doid1)
        appendMeta(dg, parent=doid0, zone=9800) # New parent
        appendMeta(dg, parent=INVALID_DO_ID, zone=INVALID_ZONE) # Old parent
        self.expect(conn, dg)

        # Ignore already tested behavior
        conn.remove_channel(230)
        location0.flush()
        obj0.flush()

        # Remove Owner
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_channel(0)
        conn.send(dg)
        conn.flush()
        obj1.flush()



        ### Test for SetLocation with a ram-broadcast field ### (continues from previous)
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(doid1)
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
        dg = Datagram.create([doid2<<ZONE_SIZE_BITS|9900], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, doid2, 9900, DistributedTestObject1)
        dg.add_uint32(43214321) # setRequired1
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("The Cutest Thing Ever!")
        self.expect(location2, dg)



        ### Test for SetLocation with a non-broadcast ram field ### (continues from previous)
        # Delete the first object for reuse
        deleteObject(conn, 5, doid1)
        obj1.flush() # Ignore received message
        obj2.flush() # Ignore parent notification
        location2.flush() # Ignore location notification

        # Recreate the first object with a ram, non-broadcast field.
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
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
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|9800], doid1,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, doid0, 9800, DistributedTestObject3)
        dg.add_uint32(44004400) # setRequired1
        dg.add_uint32(88008800) # setRDB3
        dg.add_uint16(0) # Optional fields: 0
        self.expect(location0, dg)


        ### Clean up ###
        for doid in [doid1, doid2]:
            deleteObject(conn, 5, doid)
        for mdconn in [conn, obj0, obj1, obj2, location0, location2]:
            self.disconnect(mdconn)


    # Tests stateserver handling of DistributedClassInheritance
    def test_inheritance(self):
        self.flush_failed()
        conn = self.connect(67000<<ZONE_SIZE_BITS|2000)

        ### Test for CreateObject on a subclass ###
        # Create a DTO3, which inherits from DTO1.
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 110000000, 67000, 2000, DistributedTestObject3)
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        conn.send(dg)

        # Does it show up right?
        dg = Datagram.create([67000<<ZONE_SIZE_BITS|2000], 110000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 110000000, 67000, 2000, DistributedTestObject3)
        dg.add_uint32(12123434) # setRequired1
        dg.add_uint32(0xC0FFEE) # setRDB3
        self.expect(conn, dg)

        # Cool, now let's test the broadcast messages:
        for field in [setRDB3, setRequired1]:
            dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_doid(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            conn.send(dg)
            dg = Datagram.create([67000<<ZONE_SIZE_BITS|2000], 5, STATESERVER_OBJECT_SET_FIELD)
            dg.add_doid(110000000)
            dg.add_uint16(field)
            dg.add_uint32(0x31415927)
            self.expect(conn, dg)

        # This message is NOT part of DTO1/3 and should fail.
        # Bonus points for logging an ERROR log.
        dg = Datagram.create([110000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(110000000)
        dg.add_uint16(setB2)
        dg.add_uint32(0x11225533)
        conn.send(dg)
        self.expectNone(conn)

        ### Cleanup ###
        deleteObject(conn, 5, 110000000)
        self.disconnect(conn)

    # Tests handling of various erroneous/bad/invalid messages
    def test_error(self):
        self.flush_failed()
        conn = self.connect(5)
        conn.add_channel(80000<<ZONE_SIZE_BITS|1234)

        ### Test for updating an invalid field ###
        # Create a regular object, this is not an error...
        createEmptyDTO1(conn, 5, 234000000, 80000, 1234, 819442)

        # The object should announce its entry to its location...
        dg = Datagram.create([80000<<ZONE_SIZE_BITS|1234], 234000000,
                STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 234000000, 80000, 1234, DistributedTestObject1)
        dg.add_uint32(819442) # setRequired1
        self.expect(conn, dg)

        # Send it an update on a bad field...
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(234000000)
        dg.add_uint16(0x1337)
        dg.add_uint32(0)
        conn.send(dg)

        # Nothing should happen and the SS should log an error.
        self.expectNone(conn)


        ### Test for updating with a truncated field ### (continues from previous)
        # How about a truncated update on a valid field?
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(234000000)
        dg.add_uint16(setRequired1)
        dg.add_uint16(0) # Whoops, 16-bit instead of 32-bit!
        conn.send(dg)

        # Nothing should happen + error.
        self.expectNone(conn)


        ### Test for creating an object with an in-use doid ### (continues from previous)
        # Let's try creating it again.
        createEmptyDTO1(conn, 5, 234000000, 80000, 1234, 1234567)

        # NOTHING SHOULD HAPPEN - additionally, the SS should log either an
        # error or a warning
        self.expectNone(conn)

        # Sanity check, make sure neither of the last three tests have changed any values
        dg = Datagram.create([234000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF337) # Context
        dg.add_doid(234000000) # Id
        conn.send(dg)

        # Values should be unchanged
        dg = Datagram.create([5], 234000000, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0xF337) # Context
        appendMeta(dg, 234000000, 80000, 1234, DistributedTestObject1)
        dg.add_uint32(819442) # setRequired1
        dg.add_uint16(0) # Optional fields: 0
        self.expect(conn, dg)


        ### Test for creating an object without one if its required fields ###
        # Let's try making another one, but we'll forget the setRequired1.
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 235000000, 80000, 1234, DistributedTestObject1)
        conn.send(dg)

        # Once again, nothing should happen and the SS should log an ERROR.
        self.expectNone(conn)

        # Additionally, no object should be received on a get all
        dg = Datagram.create([235000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF448) # Context
        dg.add_doid(235000000) # Id
        conn.send(dg)

        # Values should be unchanged
        self.expectNone(conn)


        ### Test for creating an object with an unrecognized DistributedClass ###
        # Let's try making a bad object.
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 236000000, 80000, 1234, 0x1337)
        conn.send(dg)

        # Nothing should happen and the SS should log an ERROR.
        self.expectNone(conn)

        # Additionally, no object should be received on a get all
        dg = Datagram.create([236000000], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0xF559) # Context
        dg.add_doid(236000000) # Id
        conn.send(dg)

        # Values should be unchanged
        self.expectNone(conn)

        ### Cleanup ###
        deleteObject(conn, 5, 234000000)
        self.disconnect(conn)

    def test_single_object_accessors(self):
        self.flush_failed()
        conn = self.connect(5)
        doid = 0x99887766
        context = 0x1111
        createEmptyDTO1(conn, 5, doid, required1=0x828)

        ### Test GetField won't respond when given incorrect doid ###
        # Send a GetField request with a bad do_id
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(context)
        dg.add_doid(0xF00)
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect no response
        self.expectNone(conn) # should not recieve GetFieldResp
        context += 1


        ### Test GetFields won't respond when given incorrect doid ###
        # Send a GetFields request with a bad do_id
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(context)
        dg.add_doid(0xF00)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint16(setBR1)
        conn.send(dg)

        # Expect no response
        self.expectNone(conn) # should not recieve GetFieldsResp
        context += 1


        ### Test GetAll won't respond when given incorrect doid ###
        # Send a GetAll request with a bad do_id
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(context)
        dg.add_doid(0xF00)
        conn.send(dg)
        context += 1

        # Expect no response
        self.expectNone(conn) # should not recieve GetAllResp


        ### Test SetField won't update when given incorrect doid ###
        # Send a SetField request with a bad do_id
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(0xF00)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xF00BA5)
        conn.send(dg)

        # Ask for the value back
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xB1112) # Context
        dg.add_doid(doid)
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect the value is unchanged
        dg = Datagram.create([5], doid, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xB1112) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x828)
        self.expect(conn, dg)



        ### Test SetFields won't update when given incorrect doid ###
        # Send a SetFields request with a bad do_id
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(0xF00)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xF00BA5)
        dg.add_uint16(setBR1)
        dg.add_string("Emancipation!")
        conn.send(dg)

        # Ask for the value back
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xB1113) # Context
        dg.add_doid(doid)
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect the value is unchanged
        dg = Datagram.create([5], doid, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xB1113) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x828)
        self.expect(conn, dg)

    # Tests the message CREATE_OBJECT_WITH_REQUIRED_OTHER
    def test_create_with_other(self):
        self.flush_failed()
        conn = self.connect(90000<<ZONE_SIZE_BITS|4321)

        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545630000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        conn.send(dg)

        # We should get an ENTERZONE_WITH_REQUIRED_OTHER...
        dg = Datagram.create([90000<<ZONE_SIZE_BITS|4321], 545630000,
                             STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545630000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Cupcakes, so sweet and tasty...")
        self.expect(conn, dg)

        # If we try to include a non-ram as OTHER...
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, 545640000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        dg.add_uint16(1) # Extra fields: 1
        dg.add_uint16(setB1)
        dg.add_uint8(42)
        conn.send(dg)

        # ...the object should show up, but without the non-RAM field.
        # Additionally, an ERROR should be logged.
        dg = Datagram.create([90000<<ZONE_SIZE_BITS|4321], 545640000,
                             STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 545640000, 90000, 4321, DistributedTestObject1)
        dg.add_uint32(2099) # setRequired1
        self.expect(conn, dg)


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
            self.expect(conn, dg)

            # Found the object! Now clean it up.
            deleteObject(conn, 5, doid)

        ### Cleanup ###
        self.disconnect(conn)

    # Tests for DELETE_AI_OBJECTS
    def test_delete_ai_objects(self):
        self.flush_failed()
        conn = self.connect(62222<<ZONE_SIZE_BITS|125)

        # Create an object...
        createEmptyDTO1(conn, 5, 201, 62222, 125, 6789)

        # ... with an AI channel...
        dg = Datagram.create([201], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_channel(31337)
        conn.send(dg)

        # Ignore any noise...
        conn.flush()

        # Now let's try hitting the SS with a reset for the wrong AI:
        dg = Datagram.create([100100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_channel(41337)
        conn.send(dg)

        # Nothing should happen:
        self.expectNone(conn)

        # Now the correct AI:
        dg = Datagram.create([100100], 5, STATESERVER_DELETE_AI_OBJECTS)
        dg.add_channel(31337)
        conn.send(dg)

        # Then object should die:
        dg = Datagram.create([62222<<ZONE_SIZE_BITS|125, 31337], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(201)
        self.expect(conn, dg)

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
        dg.add_uint32(0x600DF00D) # Context
        dg.add_doid(15000) # Doid
        conn.send(dg)

        # Expect all data in response
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0x600DF00D)
        appendMeta(dg, 15000, 4, 2, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        dg.add_uint16(0) # Optional fields: 0
        self.expect(conn, dg)



        ### Test for GetField on a valid field with a set value ### (continues from previous)
        # Now get just a single field
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect get field response
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        self.expect(conn, dg)

        ### Test for GetField on a valid field with no set value ### (continues from previous)
        # Get a field not present on the object...
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0xBAD5EED) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(setBR1)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0xBAD5EED) # Context
        dg.add_uint8(FAILURE)
        self.expect(conn, dg)


        ### Test for GetField on an invalid field ### (continues from previous)
        # Get a field that is invalid for the object...
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(0x5EE5BAD) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(0x1337) # Field id
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(0x5EE5BAD) # Context
        dg.add_uint8(FAILURE)
        self.expect(conn, dg)


        ### Test for GetFields on fields with set values ### (continues from previous)
        # First lets set a second field
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(15000) # ID
        dg.add_uint16(setBR1)
        dg.add_string("MY HAT IS AWESOME!!!")
        conn.send(dg)

        # Now let's try getting multiple fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x600D533D)
        dg.add_doid(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint16(setBR1)
        conn.send(dg)

        # Expect both fields in response
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x600D533D) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint32(0)
        dg.add_uint16(setBR1)
        dg.add_string("MY HAT IS AWESOME!!!")
        self.expect(conn, dg)


        ### Test for GetFields with mixed set and unset fields ### (continues from previous)
        # Send GetFields with mixed set and unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x711E644E) # Context
        dg.add_doid(15000) # ID
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
        self.expect(conn, dg)


        ### Test for GetFields with only unset fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x822F755F) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setBRA1)
        dg.add_uint16(setBRO1)
        conn.send(dg)

        # Expect success with no fields back
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x822F755F) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(0) # Field count
        self.expect(conn, dg)


        ### Test for GetFields with only invalid fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x5FFC422C) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint16(setDb3)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x5FFC422C) # Context
        dg.add_uint8(FAILURE)
        self.expect(conn, dg)


        ### Test for GetFields with mixed valid and invalid fields ### (continues from previous)
        # Send GetFields with unset fields
        dg = Datagram.create([15000], 890, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0x4EEB311B) # Context
        dg.add_doid(15000) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint16(setRequired1)
        conn.send(dg)

        # Expect failure
        dg = Datagram.create([890], 15000, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0x4EEB311B) # Context
        dg.add_uint8(FAILURE)
        self.expect(conn, dg)


        ### Cleanup ###
        deleteObject(conn, 5, 15000)
        self.disconnect(conn)

    # Tests the message SET_FIELDS
    def test_set_fields(self):
        self.flush_failed()
        conn = self.connect(5985858)
        location = self.connect(12512<<ZONE_SIZE_BITS|66161)

        ### Test for SetFields with broadcast and ram filds ###
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 55555, 12512, 66161, DistributedTestObject3)
        dg.add_uint32(0) # setRequired1
        dg.add_uint32(0) # setRDB3
        conn.send(dg)

        # Ignore the entry message...
        location.flush()

        # Send a multi-field update with two broadcast fields and one ram.
        dg = Datagram.create([55555], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(55555) # ID
        dg.add_uint16(3) # 3 fields:
        dg.add_uint16(setDb3)
        dg.add_string('Time is candy')
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        conn.send(dg)

        # Verify that the broadcasts go out, in order... (these must be in the correct order)
        dg = Datagram.create([12512<<ZONE_SIZE_BITS|66161], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(55555) # ID
        dg.add_uint16(2) # 2 fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xD00D)
        dg.add_uint16(setB1)
        dg.add_uint8(118)
        # TODO: Implement partial field broadcasts, then uncomment the test
        #self.expect(location, dg)
        location.flush()

        # Get the ram fields to make sure they're set
        dg = Datagram.create([55555], 5985858, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0x5111) # Context
        dg.add_doid(55555) #ID
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
        self.expect(conn, dg)

        ### Cleanup ###
        deleteObject(conn, 5985858, 55555)
        self.disconnect(location)
        self.disconnect(conn)

    # Tests stateserver handling of the 'ownrecv' keyword
    def test_ownrecv(self):
        self.flush_failed()
        conn = self.connect(14253648)
        doid = 77878788
        owner = 14253648
        location = 88833<<ZONE_SIZE_BITS|99922

        ### Test for broadcast of empty
        # Create an object
        createEmptyDTO1(conn, 5, doid, 88833, 99922, 0)

        # Set the object's owner
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_channel(owner)
        conn.send(dg)

        # Ignore EnterOwner message, not testing that here
        conn.flush()

        # Set field on an ownrecv message...
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(doid)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        conn.send(dg)

        # See if our owner channel gets it.
        dg = Datagram.create([owner, location], 5,
                             STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(doid)
        dg.add_uint16(setBRO1)
        dg.add_uint32(0xF005BA11)
        self.expect(conn, dg)

        # Delete the object...
        deleteObject(conn, 5, doid)

        # And expect to be notified
        dg = Datagram.create([owner, location], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid)
        self.expect(conn, dg)

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
        dg.add_channel(owner1chan)
        conn.send(dg)

        # ... and see if it enters the owner.
        dg = Datagram.create([owner1chan], doid1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        self.expect(owner1, dg)



        ### Test for SetOwner on an object with an existing owner ### (continues from previous)
        # Change owner to someone else...
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_OWNER)
        dg.add_channel(owner2chan)
        conn.send(dg)

        # ... expecting a changing owner ...
        dg = Datagram.create([owner1chan], 5, STATESERVER_OBJECT_CHANGING_OWNER)
        dg.add_doid(doid1)
        dg.add_channel(owner2chan) # New owner
        dg.add_channel(owner1chan) # Old owner
        self.expect(owner1, dg)
        # ... and see if it enters the new owner.
        dg = Datagram.create([owner2chan], doid1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        appendMeta(dg, doid1, 0, 0, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        self.expect(owner2, dg)



        ### Test for SetOwner on an object with owner fields and non-owner, non-broadcast fields ###
        # Create an object with a bunch of types of fields
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
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
        dg.add_channel(owner1chan)
        conn.send(dg)

        # ... and see if it enters the owner with only broadcast and/or ownrecv fields.
        dg = Datagram.create([owner1chan], doid2, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_doid(doid2) # Id
        dg.add_channel(0) # Location
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(0) # setRequired1
        dg.add_uint32(0) # setRDB3
        dg.add_uint16(2) # Optional fields: 2
        dg.add_uint16(setBR1)
        dg.add_string("I feel radder, faster... more adequate!")
        dg.add_uint16(setBRO1)
        dg.add_uint32(0x1337)
        self.expect(owner1, dg)

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
        location0 = self.connect(88<<ZONE_SIZE_BITS|99)

        ### Test for broadcast of a molecular SetField is molecular ###
        # Create an object
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        conn.send(dg)

        # Verify its entry...
        dg = Datagram.create([88<<ZONE_SIZE_BITS|99], 73317331,
                             STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(13) # setX
        dg.add_uint32(37) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(11) # setZ
        self.expect(location0, dg)

        # Send a molecular update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        conn.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<ZONE_SIZE_BITS|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(73317331)
        dg.add_uint16(setXyz)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(77) # setZ
        self.expect(location0, dg)



        ### Test for molecular SetField properly updating the individual values
        ### (continues from previous)
        # Look at the object and see if the requireds are updated...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(0) # Context
        dg.add_doid(73317331)
        conn.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(0) # Context
        appendMeta(dg, 73317331, 88, 99, DistributedTestObject4)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(77) # setZ
        dg.add_uint16(0) # Optional fields: 0
        self.expect(conn, dg)



        ### Test for molecular SetField with a ram, not-required, fields
        ### (continues from previous)
        # Now try a RAM update...
        dg = Datagram.create([73317331], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        conn.send(dg)

        # See if the MOLECULAR (not the individual fields) is broadcast.
        dg = Datagram.create([88<<ZONE_SIZE_BITS|99], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(73317331)
        dg.add_uint16(set123)
        dg.add_uint8(1) # setOne
        dg.add_uint8(2) # setTwo
        dg.add_uint8(3) # setThree
        self.expect(location0, dg)

        # A query should have all of the individual fields, not the molecular.
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(1) # Context
        dg.add_doid(73317331) # ID
        conn.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(1) # Context
        dg.add_doid(73317331) # Id
        dg.add_doid(88) # Parent
        dg.add_doid(99) # Zone
        dg.add_uint16(DistributedTestObject4)
        dg.add_uint32(55) # setX
        dg.add_uint32(66) # setY
        dg.add_uint32(999999) # setUnrelated
        dg.add_uint32(77) # setZ
        dg.add_uint16(3) # Optional fields: 3
        dg.add_uint16(setOne)
        dg.add_uint8(1)
        dg.add_uint16(setTwo)
        dg.add_uint8(2)
        dg.add_uint16(setThree)
        dg.add_uint8(3)
        self.expect(conn, dg)


        ### Test for GetFields on mixed molecular and atomic fields ### (continues from previous)
        # Now let's test querying individual fields...
        dg = Datagram.create([73317331], 13371337, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_doid(73317331) # ID
        dg.add_uint16(5) # Field count: 5
        dg.add_uint16(setXyz)
        dg.add_uint16(setOne)
        dg.add_uint16(setUnrelated)
        dg.add_uint16(set123)
        dg.add_uint16(setX)
        conn.send(dg)

        dg = Datagram.create([13371337], 73317331, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(0xBAB55EED) # Context
        dg.add_uint8(SUCCESS) # Resp status
        dg.add_uint16(5) # Field count: 5
        dg.add_uint16(setX)
        dg.add_uint32(55)
        dg.add_uint16(setUnrelated)
        dg.add_uint32(999999)
        dg.add_uint16(setXyz)
        dg.add_uint32(55)
        dg.add_uint32(66)
        dg.add_uint32(77)
        dg.add_uint16(setOne)
        dg.add_uint8(1)
        dg.add_uint16(set123)
        dg.add_uint8(1)
        dg.add_uint8(2)
        dg.add_uint8(3)
        self.expect(conn, dg)

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

        # Convenience function:
        def checkObjects(objects, zones):
            # Mitigate race condition - allow SS time to process recent changes
            time.sleep(0.1)

            # 1. Send query:
            dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_GET_ZONES_OBJECTS)
            dg.add_uint32(0xF337) # Context
            dg.add_doid(doid0) # Parent Id
            dg.add_uint16(len(zones)) # Zone count
            for zone in zones:
                dg.add_zone(zone)
            conn.send(dg)

            # 2. Expect object count:
            expected = []
            dg = Datagram.create([5], doid0, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
            dg.add_uint32(0xF337) # Context
            dg.add_doid(len(objects)) # Count of objects
                                      # Note: Using doid because range of object-count matches total doid size
            expected.append(dg)

            # 3. Expect objects:
            for id, zone in objects:
                dg = Datagram.create([5], id, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
                dg.add_uint32(0xF337)
                appendMeta(dg, id, doid0, zone, DistributedTestObject1)
                dg.add_uint32(0) # setRequired1
                expected.append(dg)

            self.expectMany(conn, expected)

            # Shouldn't receive messages from any of the other objects
            self.expectNone(conn)

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
        checkObjects([(doid1, 912), (doid2, 912), (doid3, 930)], [912, 930])

        ### Test for proper updating ###

        # Let's move doid4 in there:
        dg = Datagram.create([doid4], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid0, zone=930)
        conn.send(dg)

        checkObjects([(doid1, 912), (doid2, 912), (doid3, 930), (doid4, 930)], [912, 930])

        # Delete doid2:
        deleteObject(conn, 5, doid2)
        checkObjects([(doid1, 912), (doid3, 930), (doid4, 930)], [912, 930])

        # Move doid3 away:
        dg = Datagram.create([doid3], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=doid1, zone=930)
        conn.send(dg)
        checkObjects([(doid1, 912), (doid4, 930)], [912, 930])

        ### Test for proper OBJECT_LOCATION_ACK behavior ###

        # This parent isn't active on any stateserver, which allows us to control the
        # parent's acknowledgement of its own children.
        parent = 1824

        # Let's move doid4 and doid5 into some zone under our parent
        dg = Datagram.create([doid4, doid5], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=parent, zone=1763)
        conn.send(dg)

        # Next, as the parent, we'll acknowledge doid5
        dg = Datagram.create([doid5], parent, STATESERVER_OBJECT_LOCATION_ACK)
        dg.add_doid(parent)
        dg.add_zone(1763)
        conn.send(dg)

        # We'll acknowledge doid4, but with the old zone:
        dg = Datagram.create([doid4], parent, STATESERVER_OBJECT_LOCATION_ACK)
        dg.add_doid(parent)
        dg.add_zone(930)
        conn.send(dg)

        # Now we fake a relayed parent->child query. There's no use sending a query to
        # the parent object, because we essentially are the parent object.
        dg = Datagram.create([PARENT_PREFIX | parent], 5, STATESERVER_OBJECT_GET_ZONE_OBJECTS)
                                                          # Use the singular message so
                                                          # that it's covered in a test
        dg.add_uint32(0xBEEF) # Context
        dg.add_doid(parent) # Parent Id
        dg.add_zone(1763)
        conn.send(dg)

        expected = []
        # doid5 should respond with a contextual entry
        dg = Datagram.create([5], doid5, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(0xBEEF) # context
        appendMeta(dg, doid5, parent, 1763, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        expected.append(dg)

        # doid4 should just respond with an ENTER_LOCATION
        dg = Datagram.create([5], doid4, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid4, parent, 1763, DistributedTestObject1)
        dg.add_uint32(0) # setRequired1
        expected.append(dg)

        # Now make sure the objects responded accordingly
        self.expectMany(conn, expected)
        # And make sure nobody else responded
        self.expectNone(conn)

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
        location1 = self.connect(doid0<<ZONE_SIZE_BITS|710)
        location20 = self.connect(doid0<<ZONE_SIZE_BITS|720)
        location21 = self.connect(doid1<<ZONE_SIZE_BITS|720)

        ### Test for ObjectDeleteChildren ###
        # Create an object tree
        createEmptyDTO1(conn, 5, doid0)
        createEmptyDTO1(conn, 5, doid1, doid0, 710)
        createEmptyDTO1(conn, 5, doid2, doid0, 720)

        # Ignore entry broadcasts
        location1.flush()
        location20.flush()
        children0.flush()

        # Send delete children
        dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_doid(doid0)
        conn.send(dg)

        # Expect children to receive delete children...
        dg = Datagram.create([PARENT_PREFIX|doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_doid(doid0)
        self.expect(children0, dg)
        # ... and then broadcast their own deletion
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|710], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid1)
        self.expect(location1, dg)
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|720], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid2)
        self.expect(location20, dg)



        ### Test for DeleteRam propogation to children ###
        # Add our children in multi-tier tree
        createEmptyDTO1(conn, 5, doid1, doid0, 710)
        createEmptyDTO1(conn, 5, doid2, doid1, 720)

        # Ignore entry broadcasts
        location1.flush()
        location21.flush()

        # Delete the root object
        dg = Datagram.create([doid0], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid0)
        conn.send(dg)

        # Expect the first object to receive delete children from object zero...
        dg = Datagram.create([PARENT_PREFIX|doid0], 5, STATESERVER_OBJECT_DELETE_CHILDREN)
        dg.add_doid(doid0)
        self.expect(children0, dg)
        # ... and delete itself...
        dg = Datagram.create([doid0<<ZONE_SIZE_BITS|710], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid1)
        self.expect(location1, dg)
        # ... and propogate the deletion to its child...
        dg = Datagram.create([doid1<<ZONE_SIZE_BITS|720], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid2)
        self.expect(location21, dg)


        ### Cleanup ###
        for mdconn in [conn, children0, location1, location20, location21]:
            self.disconnect(mdconn)

    def test_active_zones(self):
        self.flush_failed()
        conn = self.connect(5)

        doid0 = 1000 # root
        doid1 = 2000
        doid2 = 2001

        # create a tree
        createEmptyDTO1(conn, 5, doid0)
        createEmptyDTO1(conn, 5, doid1, doid0, 1234)
        createEmptyDTO1(conn, 5, doid2, doid0, 1337)

        time.sleep(0.3)

        dg = Datagram.create([doid0], 5, STATESERVER_GET_ACTIVE_ZONES)
        dg.add_uint32(0)
        conn.send(dg)

        dg = Datagram.create([5], doid0, STATESERVER_GET_ACTIVE_ZONES_RESP)
        dg.add_uint32(0)
        dg.add_uint16(2)
        dg.add_zone(1234)
        dg.add_zone(1337)
        self.expect(conn, dg)

        ### Cleanup ###
        deleteObject(conn, 5, doid0)
        deleteObject(conn, 5, doid1)
        deleteObject(conn, 5, doid2)
        self.disconnect(conn)

    # Tests the keyword clrecv
    def test_clrecv(self):
        self.flush_failed()
        conn = self.connect(0xBAD << ZONE_SIZE_BITS | 0xF001)

        doid1 = 0xF00

        ### Test for clrecv keyword ###
        # Created a distributed chunk
        dg = Datagram.create([100100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 0xBAD, 0xF001, DistributedChunk)
        dg.add_size(12) # blockList size in bytes (contains 1 element)
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
        dg = Datagram.create([0xBAD << ZONE_SIZE_BITS | 0xF001], doid1,
            STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 0xBAD, 0xF001, DistributedChunk)
        dg.add_size(12) # blockList size in bytes (contains 1 element)
        dg.add_uint32(43) # blockList[0].x
        dg.add_uint32(54) # blockList[0].y
        dg.add_uint32(65) # blockList[0].z
        dg.add_uint16(1) # Optional fields: 1
        dg.add_uint16(lastBlock)
        dg.add_uint32(0) # lastBlock.x
        dg.add_uint32(0) # lastBlock.y
        dg.add_uint32(0) # lastBlock.z
        self.expect(conn, dg)

        # Update the object with some new values
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(doid1) # Id
        dg.add_uint16(3) # Num fields: 3
        dg.add_uint16(blockList)
        dg.add_size(24) # blockList size in bytes (contains 2 elements)
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
        dg = Datagram.create([0xBAD << ZONE_SIZE_BITS | 0xF001], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(doid1) # Id
        dg.add_uint16(newBlock)
        dg.add_uint32(171) # lastBlock.x
        dg.add_uint32(282) # lastBlock.y
        dg.add_uint32(393) # lastBlock.z
        self.expect(conn, dg)


        ### Cleanup ###
        deleteObject(conn, 5, doid1)
        self.disconnect(conn)

if __name__ == '__main__':
    unittest.main()
