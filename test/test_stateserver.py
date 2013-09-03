#!/usr/bin/env python2
import unittest
from socket import *

from common import *
from test_dc import *

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

class TestStateServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        cls.c = MDConnection(c)

    @classmethod
    def tearDownClass(cls):
        cls.c.close()
        cls.daemon.stop()

    def test_create_destroy(self):
        self.c.flush()
        self.c.send(Datagram.create_add_channel(5000<<32|1500))

        # Create a DistributedTestObject1...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(101000000) # ID
        dg.add_uint32(6789) # setRequired1
        self.c.send(dg)

        # The object should announce its entry to the zone-channel...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(101000000) # ID
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Destroy our object...
        dg = Datagram.create([101000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        self.c.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        self.assertTrue(self.c.expect(dg))

        # We're done here...
        self.c.send(Datagram.create_remove_channel(5000<<32|1500))

    def test_broadcast(self):
        self.c.flush()
        self.c.send(Datagram.create_add_channel(5000<<32|1500))

        # Create a DistributedTestObject2...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject2)
        dg.add_uint32(101000005) # ID
        self.c.send(dg)

        # Ignore the entry message, we aren't testing that here.
        self.c.flush()

        # Hit it with an update on setB2.
        dg = Datagram.create([101000005], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        self.c.send(dg)

        # Object should broadcast that update.
        # N.B. the who field is not a mistake. This is so AI servers can see
        # who the update ultimately comes from for e.g. an airecv/clsend.
        dg = Datagram.create([5000<<32|1500], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(101000005)
        dg.add_uint16(setB2)
        dg.add_uint32(0x31415927)
        self.assertTrue(self.c.expect(dg))

        # Clean up.
        dg = Datagram.create([101000005], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000005)
        self.c.send(dg)
        self.c.send(Datagram.create_remove_channel(5000<<32|1500))

    def test_airecv(self):
        self.c.flush()

        ai1 = 500
        ai2 = 501
        obj1 = 12345
        obj2 = 123456
        obj3 = 1234567
        obj4 = 1010101

        # So we can see airecvs...
        self.c.send(Datagram.create_add_channel(ai1))
        self.c.send(Datagram.create_add_channel(ai2))
        # So we can see communications between objects...
        self.c.send(Datagram.create_add_channel(obj1))
        self.c.send(Datagram.create_add_channel(4030<<32|obj1))
        self.c.send(Datagram.create_add_channel(obj2))
        self.c.send(Datagram.create_add_channel(4030<<32|obj2))
        self.c.send(Datagram.create_add_channel(obj3))
        self.c.send(Datagram.create_add_channel(4030<<32|obj3))
        self.c.send(Datagram.create_add_channel(obj4))

        # Create our first object...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj1) # ID
        dg.add_uint32(6789) # setRequired1
        self.c.send(dg)

        # First object belongs to AI1...
        dg = Datagram.create([obj1], 5, STATESERVER_OBJECT_SET_AI_CHANNEL)
        dg.add_uint32(obj1)
        dg.add_uint64(ai1)
        self.c.send(dg)

        # Object should announce its presence to AI1...
        dg_ai = Datagram.create([ai1], obj1, STATESERVER_OBJECT_ENTER_AI_RECV)
        dg_ai.add_uint32(5000) # Parent
        dg_ai.add_uint32(1500) # Zone
        dg_ai.add_uint16(DistributedTestObject1)
        dg_ai.add_uint32(obj1)
        dg_ai.add_uint32(6789) # setRequired1
        dg_ai.add_uint16(0) # Number of extra fields embedded in message: none
        # ...and AI1 to its children...
        dg_children = Datagram.create([4030<<32|obj1], obj1, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg_children.add_uint32(obj1)
        dg_children.add_uint64(ai1)
        self.assertTrue(self.c.expect_multi([dg_ai, dg_children]))

        # Next, let's create a second object beneath it.
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(obj1) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj2) # ID
        dg.add_uint32(1337) # setRequired1
        self.c.send(dg)

        # The second object should ask its parent for the AI channel...
        dg_query = Datagram.create([obj1], obj2, STATESERVER_OBJECT_QUERY_MANAGING_AI)

        # To which the parent should reply...
        dg_notify = Datagram.create([obj2], obj1, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg_notify.add_uint32(obj1)
        dg_notify.add_uint64(ai1)

        # Then the SECOND object should announce its presence to AI1...
        dg_ai = Datagram.create([ai1], obj2, STATESERVER_OBJECT_ENTER_AI_RECV)
        dg_ai.add_uint32(obj1) # Parent
        dg_ai.add_uint32(1500) # Zone
        dg_ai.add_uint16(DistributedTestObject1)
        dg_ai.add_uint32(obj2)
        dg_ai.add_uint32(1337) # setRequired1
        dg_ai.add_uint16(0) # Number of extra fields embedded in message: none
        # ...and AI1 to its children...
        dg_children = Datagram.create([4030<<32|obj2], obj2, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg_children.add_uint32(obj2)
        dg_children.add_uint64(ai1)
        self.assertTrue(self.c.expect_multi([dg_query, dg_notify, dg_ai, dg_children]))

        # Make a third object in a different zone...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(5050) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj3) # ID
        dg.add_uint32(289855) # setRequired1
        self.c.send(dg)

        # Nothing should happen yet...
        self.assertTrue(self.c.expect_none())

        # Slide obj3 over underneath obj2...
        dg = Datagram.create([obj3], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(obj2) # Parent
        dg.add_uint32(1000) # Zone
        self.c.send(dg)

        # The third object should ask its new parent for the AI channel...
        dg_query = Datagram.create([obj2], obj3, STATESERVER_OBJECT_QUERY_MANAGING_AI)

        # To which the parent should reply...
        dg_notify = Datagram.create([obj3], obj2, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg_notify.add_uint32(obj2)
        dg_notify.add_uint64(ai1)

        # And then obj3 announces to AI and its children...
        dg_ai = Datagram.create([ai1], obj3, STATESERVER_OBJECT_ENTER_AI_RECV)
        dg_ai.add_uint32(obj2) # Parent
        dg_ai.add_uint32(1000) # Zone
        dg_ai.add_uint16(DistributedTestObject1)
        dg_ai.add_uint32(obj3)
        dg_ai.add_uint32(289855) # setRequired1
        dg_ai.add_uint16(0) # Number of extra fields embedded in message: none
        # ...and AI1 to its children...
        dg_children = Datagram.create([4030<<32|obj3], obj3, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg_children.add_uint32(obj3)
        dg_children.add_uint64(ai1)
        self.assertTrue(self.c.expect_multi([dg_query, dg_notify, dg_ai, dg_children]))

        # Test the airecv keyword...
        dg = Datagram.create([obj3], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(obj3)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.c.send(dg)

        # Now the AI should see it...
        # BONUS POINTS: The broadcast should be a separate channel of the same
        # message.
        dg = Datagram.create([ai1, (obj2<<32|1000)], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(obj3)
        dg.add_uint16(setBA1)
        dg.add_uint16(0xF00D)
        self.assertTrue(self.c.expect(dg))

        # Now, let's make obj4 on ai2.
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(3141) # Parent
        dg.add_uint32(1234) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj4) # ID
        dg.add_uint32(0xDECAFBAD) # setRequired1
        self.c.send(dg)
        dg = Datagram.create([obj4], 5, STATESERVER_OBJECT_SET_AI_CHANNEL)
        dg.add_uint32(obj4)
        dg.add_uint64(ai2)
        self.c.send(dg)

        # We don't care about the ai2 announcement; that's been tested already.
        self.c.flush()

        # Now, let's pick up obj2 and move it under obj4.
        dg = Datagram.create([obj2], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(obj4) # Parent
        dg.add_uint32(1000) # Zone
        self.c.send(dg)

        # AMONG THE DELUGE OF MESSAGES WE RECEIVE (NOT NECESSARILY IN ORDER):
        expected = []
        # obj2 tells AI1 about the zone change. Like with field updates, this
        # sets the sender field to the channel that instigated the change.
        dg = Datagram.create([ai1], 5, STATESERVER_OBJECT_CHANGE_ZONE)
        dg.add_uint32(obj2)
        dg.add_uint32(obj4)
        dg.add_uint32(1000)
        dg.add_uint32(obj1)
        dg.add_uint32(1500)
        expected.append(dg)
        # obj2 asks obj4 for the AI channel.
        dg = Datagram.create([obj4], obj2, STATESERVER_OBJECT_QUERY_MANAGING_AI)
        expected.append(dg)
        # obj4 tells obj2 what the AI channel is.
        dg = Datagram.create([obj2], obj4, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg.add_uint32(obj4)
        dg.add_uint64(ai2)
        expected.append(dg)
        # obj2 tells its children that there's a new AI channel.
        dg = Datagram.create([4030<<32|obj2], obj2, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg.add_uint32(obj2)
        dg.add_uint64(ai2)
        expected.append(dg)
        # obj3 tells its children as well.
        dg = Datagram.create([4030<<32|obj3], obj3, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg.add_uint32(obj3)
        dg.add_uint64(ai2)
        expected.append(dg)
        # obj2 announces its entry into ai2's airspace...
        dg = Datagram.create([ai2], obj2, STATESERVER_OBJECT_ENTER_AI_RECV)
        dg.add_uint32(obj4) # Parent
        dg.add_uint32(1000) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj2)
        dg.add_uint32(1337) # setRequired1
        dg.add_uint16(0) # Number of extra fields embedded in message: none
        expected.append(dg)
        # ...and its departure from ai1's.
        dg = Datagram.create([ai1], obj2, STATESERVER_OBJECT_LEAVING_AI_INTEREST)
        dg.add_uint32(obj2)
        expected.append(dg)
        # obj3 does the same.
        dg = Datagram.create([ai2], obj3, STATESERVER_OBJECT_ENTER_AI_RECV)
        dg.add_uint32(obj2) # Parent
        dg.add_uint32(1000) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(obj3)
        dg.add_uint32(289855) # setRequired1
        dg.add_uint16(0) # Number of extra fields embedded in message: none
        expected.append(dg)
        # + departure:
        dg = Datagram.create([ai1], obj3, STATESERVER_OBJECT_LEAVING_AI_INTEREST)
        dg.add_uint32(obj3)
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Now let's test the verification of the AI channel notification system.

        # A notification with a not-current parent should do nothing:
        dg = Datagram.create([obj3], obj1, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg.add_uint32(obj1)
        dg.add_uint64(ai1)
        self.c.send(dg)
        self.assertTrue(self.c.expect_none())

        # A notification with the same AI channel should also do nothing:
        dg = Datagram.create([obj3], obj2, STATESERVER_OBJECT_NOTIFY_MANAGING_AI)
        dg.add_uint32(obj2)
        dg.add_uint64(ai2)
        self.c.send(dg)
        self.assertTrue(self.c.expect_none())

        # Cleanup time:
        self.c.send(Datagram.create_remove_channel(ai1))
        self.c.send(Datagram.create_remove_channel(ai2))
        self.c.send(Datagram.create_remove_channel(obj1))
        self.c.send(Datagram.create_remove_channel(4030<<32|obj1))
        self.c.send(Datagram.create_remove_channel(obj2))
        self.c.send(Datagram.create_remove_channel(4030<<32|obj2))
        self.c.send(Datagram.create_remove_channel(obj3))
        self.c.send(Datagram.create_remove_channel(4030<<32|obj3))
        self.c.send(Datagram.create_remove_channel(obj4))
        # Clean up objects:
        for obj in [obj1, obj2, obj3, obj4]:
            dg = Datagram.create([obj], 5, STATESERVER_OBJECT_DELETE_RAM)
            dg.add_uint32(obj)
            self.c.send(dg)

    def test_ram(self):
        self.c.flush()

        self.c.send(Datagram.create_add_channel(13000<<32|6800))
        self.c.send(Datagram.create_add_channel(13000<<32|4800))

        # Create an object...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(6800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(102000000) # ID
        dg.add_uint32(12341234) # setRequired1
        self.c.send(dg)

        # See if it shows up...
        dg = Datagram.create([13000<<32|6800], 102000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(6800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(102000000) # ID
        dg.add_uint32(12341234) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # At this point we don't care about zone 6800.
        self.c.send(Datagram.create_remove_channel(13000<<32|6800))

        # Hit it with a RAM update.
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(102000000)
        dg.add_uint16(setBR1)
        dg.add_string("Boots of Coolness (+20%)")
        self.c.send(dg)

        # Now move it over into zone 4800...
        dg = Datagram.create([102000000], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(13000) # Parent
        dg.add_uint32(4800) # Zone
        self.c.send(dg)

        # Verify that it announces its entry with the RAM message included.
        dg = Datagram.create([13000<<32|4800], 102000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER)
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
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(105000000) # ID
        dg.add_uint32(43214321) # setRequired1
        self.c.send(dg)

        # See if it shows up...
        dg = Datagram.create([14000<<32|9800], 105000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9800) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(105000000) # ID
        dg.add_uint32(43214321) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Now move it over into zone 9900...
        dg = Datagram.create([105000000], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(14000) # Parent
        dg.add_uint32(9900) # Zone
        self.c.send(dg)

        # See if it announces its departure from 9800...
        expected = []
        dg = Datagram.create([14000<<32|9800], 5, STATESERVER_OBJECT_CHANGE_ZONE)
        dg.add_uint32(105000000)
        dg.add_uint32(14000)
        dg.add_uint32(9900)
        dg.add_uint32(14000)
        dg.add_uint32(9800)
        expected.append(dg)
        # ...and its entry into 9900.
        dg = Datagram.create([14000<<32|9900], 105000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
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

if __name__ == '__main__':
    unittest.main()
