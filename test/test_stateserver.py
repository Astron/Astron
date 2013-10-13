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
        dg = Datagram.create([101000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        ai.send(dg)

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
        dg = Datagram.create([101000005], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000005)
        ai.send(dg)

        # Unsubscribe location
        ai.close()

    # Tests stateserver handling of airecv keyword and the GET_AI/SET_AI/CHANGING/ENTER messages.
    def test_airecv(self):
        control = ChannelConnection(5)

        # So we can see airecvs...
        ai1chan = 500
        ai2chan = 501
        ai1 = ChannelConnection(ai1chan)
        ai2 = ChannelConnection(ai2chan)

        # So we can see communications between objects...
        obj1id = 12345
        obj2id = 123456
        obj3id = 1234567
        obj4id = 1010101
        obj1 = ChannelConnection(obj1id)
        obj2 = ChannelConnection(obj2id)
        obj3 = ChannelConnection(obj3id)
        obj4 = ChannelConnection(obj4id)
        children1 = ChannelConnection(PARENT_PREFIX|obj1id)
        children2 = ChannelConnection(PARENT_PREFIX|obj2id)
        children3 = ChannelConnection(PARENT_PREFIX|obj3id)

        ### Test for SetAI which responds ChangingAI, EnterAI ###
        # Create our first object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, obj1id, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        control.send(dg)

        # First object belongs to AI1...
        dg = Datagram.create([obj1id], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai1chan)
        control.send(dg)

        # Object should announce its presence to AI1...
        dg = Datagram.create([ai1chan], obj1id, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, obj1id, 5000, 1500, DistributedTestObject1)
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(ai1.expect(dg)) # AI recieved ENTER_AI

        # ...and AI1 to its children...
        dg = Datagram.create([PARENT_PREFIX|obj1id], obj1id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint64(INVALID_CHANNEL) # Old AI
        self.assertTrue(children1.expect(dg)) # Children recieved CHANGING_AI



        ### Test for child AI handling on creation ### (continues from previous)
        # Next, let's create a second object beneath it.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, obj2id, obj1id, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        control.send(dg)

        # The second object should ask its parent for the AI channel...
        # The parent should is also expecting a CHANGING_LOCATION message we want to ignore...
        while True:
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None) # Parent recieved messages
            dgi = DatagramIterator(dg)
            msgtype = dgi.read_uint16()
            if msgtype is STATESERVER_OBJECT_GET_AI:
                break
            elif msgtype is STATESERVER_OBJECT_CHANGING_LOCATION:
                continue
            else:
                self.fail("obj1 recieved unexpected msgtype: " + str(msgtype))
        self.assertTrue(dgi.matches_header([obj1id], obj2id, STATESERVER_OBJECT_GET_AI))
        context = dgi.read_uint32()
        obj1.flush()

        # To which the parent should reply...
        dg = Datagram.create([obj2id], obj1id, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(obj1id)
        dg.add_uint64(ai1chan)
        self.assertTrue(obj2.expect(dg)) # Receiving GET_AI_RESP from parent

        # Then the SECOND object should announce its presence to AI1...
        dg = Datagram.create([ai1chan], obj2id, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, obj2id, obj1id, 1500, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        self.assertTrue(ai1.expect(dg)) # Obj2 enters AI1

        # ...and AI1 to its children...
        dg = Datagram.create([PARENT_PREFIX|obj2id], obj2id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj2id)
        dg.add_uint64(ai1chan)
        dg.add_uint64(INVALID_CHANNEL)
        self.assertTrue(children2.expect(dg)) # Receiving CHANGING_AI



        ### Test for child AI handling on reparent ### (continues from previous)
        # Make a third object in a different location...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(obj3id, 5050, 1500, DistributedTestObject1)
        dg.add_uint32(289855) # setRequired1
        control.send(dg)

        # Nothing should happen yet on the channels we listen to...
        self.assertTrue(self.ai1.expect_none())
        self.assertTrue(self.obj1.expect_none())
        self.assertTrue(self.obj2.expect_none())
        self.assertTrue(self.obj3.expect_none())

        # Slide obj3 over underneath obj2...
        dg = Datagram.create([obj3id], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendMeta(dg, parent=obj2id, zone=1000)
        control.send(dg)

        # The third object should ask its new parent for the AI channel...
        # The parent should is also expecting a CHANGING_LOCATION message we want to ignore...
        while True:
            dg = obj1.recv_maybe()
            self.assertTrue(dg is not None) # Parent recieved messages
            dgi = DatagramIterator(dg)
            msgtype = dgi.read_uint16()
            if msgtype is STATESERVER_OBJECT_GET_AI:
                break
            elif msgtype is STATESERVER_OBJECT_CHANGING_LOCATION:
                continue
            else:
                self.fail("obj2 recieved unexpected msgtype: " + str(msgtype))
        self.assertTrue(dgi.matches_header([obj2id], obj3id, STATESERVER_OBJECT_GET_AI))
        context = dgi.read_uint32()
        obj2.flush()

        # To which the parent should reply...
        dg = Datagram.create([obj3id], obj2id, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(obj2id)
        dg.add_uint64(ai1chan)
        self.assertTrue(obj3.expect(dg)) # Obj2 returns GET_AI_RESP

        # And then obj3 announces its presence to AI1...
        dg = Datagram.create([ai1chan], obj3id, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, obj3id, obj2id, 1000, DistributedTestObject1)
        dg.add_uint32(289855) # setRequired1
        self.assertTrue(ai1.expect(dg)) # Obj3 sends ENTER_AI

        # ...and AI1 to its children...
        dg = Datagram.create([PARENT_PREFIX|obj3id], obj3id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj3id)
        dg.add_uint64(ai1chan) # New AI
        dg.add_uint16(INVALID_CHANNEL) # Old AI
        self.assertTrue(children3.expect(dg)) # Receiving changing AI



        ### Test for the airecv keyword ### (continues from previous)
        dg = Datagram.create([obj3id], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(obj3id)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        control.send(dg)

        # Now the AI should see it...
        # BONUS POINTS: The broadcast should be a separate channel of the same message.
        dg = Datagram.create([ai1chan, (obj2id<<32|1000)], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(obj3id)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.assertTrue(ai1.expect(dg)) # Recieve a broadcasted SET_FIELD update



        ### Test for advanced reparenting ###
        # Now, let's make obj4 on ai2.
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        appendMeta(dg, obj4id, 3141, 1234, DistributedTestObject1)
        dg.add_uint32(0xDECAFBAD) # setRequired1
        control.send(dg)
        dg = Datagram.create([obj4id], 5, STATESERVER_OBJECT_SET_AI)
        dg.add_uint64(ai2chan)
        control.send(dg)

        # We don't care about the ai2 announcement; that's been tested already.
        ai2.flush()

        # Now, let's pick up obj2 and move it under obj4.
        dg = Datagram.create([obj2id], 5, STATESERVER_OBJECT_SET_LOCATION)
        appendData(dg, parent=obj4id, zone=1000)
        control.send(dg)

        ## AMONG THE DELUGE OF MESSAGES WE RECEIVE (NOT NECESSARILY IN ORDER) ##
        ai1expected = []
        ai2expected = []

        # obj2 tells AI1 about the location change. Like with field updates, this
        # sets the sender field to the channel that instigated the change.
        dg = Datagram.create([ai1chan], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(obj2id) # ID
        dg.add_uint32(obj4id) # New parent
        dg.add_uint32(1000) # New zone
        dg.add_uint32(obj1id) # Old parent
        dg.add_uint32(1500) # Old zone
        ai1expected.append(dg)

        # obj2 asks obj4 for the AI channel.
        dg = obj4.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([obj4id], obj2id, STATESERVER_OBJECT_GET_AI))
        context = dgi.read_uint32()
        # obj4 tells obj2 what the AI channel is.
        dg = Datagram.create([obj2id], obj4id, STATESERVER_OBJECT_GET_AI_RESP)
        dg.add_uint32(context)
        dg.add_uint32(obj4id)
        dg.add_uint64(ai2chan)
        obj2.expect(dg)

        # obj2 tells its children and old AI that there's a new AI channel.
        dg = Datagram.create([PARENT_PREFIX|obj2id, ai1chan], obj2id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj2id)
        dg.add_uint64(ai2chan) # New AI
        dg.add_uint64(ai1chan) # Old AI
        children2.expect(dg)
        ai1expected.append(dg)
        # obj3 tells its children and old AI as well.
        dg = Datagram.create([PARENT_PREFIX|obj3id, ai1chan], obj3id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj3id)
        dg.add_uint64(ai2chan) # New AI
        dg.add_uint64(ai1chan) # Old AI
        children3.expect(dg)
        ai1expected.append(dg)

        # obj2 announces its entry into ai2's airspace...
        dg = Datagram.create([ai2chan], obj2id, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(dg, obj2id, obj4id, 1000, DistributedTestObject1)
        dg.add_uint32(1337) # setRequired1
        ai2expected.append(dg)
        # obj3 does the same.
        dg = Datagram.create([ai2chan], obj3id, STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED)
        appendMeta(obj3id, obj2id, 1000, DistributedTestObject1)
        dg.add_uint32(289855) # setRequired1
        ai2expected.append(dg)

        # Expect datagrams to be recieved
        self.assertTrue(ai1.expect_multi(ai1expected, only=True))
        self.assertTrue(ai2.expect_multi(ai2expected, only=True))



        ### Test for AI messages with various corner cases ### (continues from previous)
        # Now let's test the verification of the AI channel notification system.

        # A notification with a not-current parent should do nothing:
        dg = Datagram.create([obj3id], obj1id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj1id)
        dg.add_uint64(0x17FEEF71) # New AI
        dg.add_uint64(ai1chan) # Old AI
        obj1.send(dg)
        self.assertTrue(ai1.expect_none())
        self.assertTrue(obj1.expect_none())
        self.assertTrue(children3.expect_none())

        # A notification with the same AI channel should also do nothing:
        dg = Datagram.create([obj3id], obj2id, STATESERVER_OBJECT_CHANGING_AI)
        dg.add_uint32(obj2id)
        dg.add_uint64(ai2chan)
        dg.add_uint64(ai2chan)
        obj2.send(dg)
        self.assertTrue(ai2.expect_none())
        self.assertTrue(obj2.expect_none())
        self.assertTrue(children3.expect_none())



        ### Test for AI notification of object deletions ###
        # Delete the object
        dg = Datagram.create([obj3id], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(obj3id)
        control.send(dg)

        # See if the AI recvs the delete.
        dg = Datagram.create([ai2chan], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(obj3id)
        self.assertTrue(ai2.expect(dg))


        ### Cleanup ###
        # Delete objects
        for doid in [obj1id, obj2id, obj3id, obj4id]:
            dg = Datagram.create([doid], 5, STATESERVER_OBJECT_DELETE_RAM)
            dg.add_uint32(doid)
            control.send(dg)

        # Close channels
        control.close()
        ai1.close()
        ai2.close()
        obj1.close()
        obj2.close()
        obj3.close()
        obj4.close()
        children1.close()
        children2.close()
        children3.close()

    def test_ram(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(13000<<32|6800))
        self.c.send(Datagram.create_add_channel(13000<<32|4800))

        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(6800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(102000000) # ID
        dg.add_uint32(12341234) # setRequired1
        self.c.send(dg)

        # See if it shows up...
        dg = Datagram.create([13000<<32|6800], 102000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(6800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(102000000) # ID
        dg.add_uint32(12341234) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # At this point we don't care about zone 6800.
        self.c.send(Datagram.create_remove_channel(13000<<32|6800))

        # Hit it with a RAM update.
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_uint32(102000000)
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.c.send(dg)

        # Now move it over into zone 4800...
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_LOCATION)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(4800) # Zone
        self.c.send(dg)

        # Verify that it announces its entry with the RAM message included.
        dg = Datagram.create([13000<<32|4800], 102000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(4800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(102000000) # ID
        dg.add_uint32(12341234) # setRequired1
        dg.add_uint16(1) # Other fields: 1
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(13000<<32|4800))
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(102000000)
        self.c.send(dg)

    def test_set_zone(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(14000<<32|9800))
        self.c.send(Datagram.create_add_channel(14000<<32|9900))

        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_CREATE_OBJECT_WITH_REQUIRED)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(105000000) # ID
        dg.add_uint32(43214321) # setRequired1
        self.c.send(dg)

        # See if it shows up...
        dg = Datagram.create([14000<<32|9800], 105000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(105000000) # ID
        dg.add_uint32(43214321) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Now move it over into zone 9900...
        dg = Datagram.create([105000000], 5, STATESERVER_OBJECT_SET_LOCATION)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9900) # Zone
        self.c.send(dg)

        # See if it announces its departure from 9800...
        expected = []
        dg = Datagram.create([14000<<32|9800], 5, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_uint32(105000000)
        dg.add_uint32(14000)
        dg.add_uint32(9900)
        dg.add_uint32(14000)
        dg.add_uint32(9800)
        expected.append(dg)
        # ...and its entry into 9900.
        dg = Datagram.create([14000<<32|9900], 105000000, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9900) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(105000000) # ID
        dg.add_uint32(43214321) # setRequired1
        expected.append(dg)

        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Clean up.
        self.c.send(Datagram.create_remove_channel(14000<<32|9800))
        self.c.send(Datagram.create_remove_channel(14000<<32|9900))
        dg = Datagram.create([105000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(105000000)
        self.c.send(dg)

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
