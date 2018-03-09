#!/usr/bin/env python2
import unittest
from common.unittests import ProtocolTest
from common.astron import *
from common.dcfile import *
import time

MD_CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    threaded: %s

general:
    dc_files:
        - %r

""" % (USE_THREADING, test_dc)

DBSS_CONFIG = """\
messagedirector:
    connect: 127.0.0.1:57123

general:
    dc_files:
        - %r

roles:
    - type: dbss
      database: 1200
      ranges:
          - min: 9000
            max: 9999
""" % (test_dc)

def appendMeta(datagram, doid=None, parent=None, zone=None, dclass=None):
    if doid is not None:
        datagram.add_doid(doid)
    if parent is not None:
        datagram.add_doid(parent)
    if zone is not None:
        datagram.add_zone(zone)
    if dclass is not None:
        datagram.add_uint16(dclass)

class TestDBStateServerPostremove(ProtocolTest):
    @classmethod
    def setUpClass(cls):
        cls.md_daemon = Daemon(MD_CONFIG)
        cls.md_daemon.start()

        cls.dbss_daemon = Daemon(DBSS_CONFIG)
        cls.dbss_daemon.start()

        cls.shard = cls.connectToServer()
        cls.shard.send(Datagram.create_set_con_name("Shard"))
        cls.shard.send(Datagram.create_add_channel(5))

        cls.database = cls.connectToServer()
        cls.database.send(Datagram.create_set_con_name("Database"))
        cls.database.send(Datagram.create_add_channel(1200))

    @classmethod
    def tearDownClass(cls):
        cls.database.send(Datagram.create_remove_channel(1200))
        cls.database.close()

        cls.shard.send(Datagram.create_remove_channel(5))
        cls.shard.close()

        cls.md_daemon.stop()

    def test_postremoves(self):
        self.database.flush()
        self.shard.flush()
        
        # Allocate zone channel on our 'shard' for our test objects (allocation logic taken more or less verbatim off test_dbss).
        self.shard.send(Datagram.create_add_channel(80000<<ZONE_SIZE_BITS|100))
        doid1 = 9001
        doid2 = 9002

        ### Test for Activate on Database object with other fields ###
        # Enter an object into ram from the disk by Activating it; with
        # overrides
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER)
        appendMeta(dg, doid1, 80000, 100)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2) # Two other fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x45894212)
        dg.add_uint16(setBR1)
        dg.add_string('Words are always nothing but excessive, a pure sound streams on...')
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid1) # object Id

        # Send back to the DBSS with some required values
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x12345678)
        dg.add_uint16(setRDB3)
        dg.add_uint32(3117)
        self.database.send(dg)

        # See if it announces its entry into 100.
        dg = Datagram.create([80000<<ZONE_SIZE_BITS|100], doid1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid1, 80000, 100, DistributedTestObject5)
        dg.add_uint32(0x45894212) # setRequired1
        dg.add_uint32(3117) # setRDB3
        dg.add_uint16(1) # One other field:
        dg.add_uint16(setBR1)
        dg.add_string('Words are always nothing but excessive, a pure sound streams on...')
        self.expect(self.shard, dg)

        dg = Datagram.create([doid2], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER)
        appendMeta(dg, doid2, 80000, 100)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2) # Two other fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x23491282)
        dg.add_uint16(setBR1)
        dg.add_string('If a sentimental argument void of distinction is allowed, then...')
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid2, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid2) # object Id

        # Send back to the DBSS with some required values
        dg = Datagram.create([doid2], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x12345678)  
        dg.add_uint16(setRDB3)
        dg.add_uint32(3117)   
        self.database.send(dg)

        # See if it announces its entry into 100.
        dg = Datagram.create([80000<<ZONE_SIZE_BITS|100], doid2, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        appendMeta(dg, doid2, 80000, 100, DistributedTestObject5)
        dg.add_uint32(0x23491282) # setRequired1
        dg.add_uint32(3117) # setRDB3
        dg.add_uint16(1) # One other field:
        dg.add_uint16(setBR1)
        dg.add_string('If a sentimental argument void of distinction is allowed, then...')
        self.expect(self.shard, dg)

        # Now for what we came for: The actual post-remove test logic.
        pr1 = Datagram.create([5], doid1, 12346)
        pr2 = Datagram.create([5], doid2, 6543)
        pr3 = Datagram.create([5], doid2, 7512)
        pr4 = Datagram.create([5], doid2, 4568)

        # Add a post-remove for our first object...
        dg = Datagram.create([doid1], 5, DBSS_ADD_POST_REMOVE)
        dg.add_uint32(doid1)
        dg.add_string(pr1.get_data())
        self.shard.send(dg)

        # The post-removes for doid1 are subsequently cleared.
        dg = Datagram.create([doid1], 5, DBSS_CLEAR_POST_REMOVES)
        dg.add_uint32(doid1)
        self.shard.send(dg)

        # Add the first post-remove for our second object.
        dg = Datagram.create([doid2], 5, DBSS_ADD_POST_REMOVE)
        dg.add_uint32(doid2)
        dg.add_string(pr2.get_data())
        self.shard.send(dg)

        # Add the second post-remove for our second object.
        dg = Datagram.create([doid2], 5, DBSS_ADD_POST_REMOVE)
        dg.add_uint32(doid2)
        dg.add_string(pr3.get_data())
        self.shard.send(dg)

        # Make sure we can't add post-removes for objects that do not exist on the DBSS, even if they were somehow routed to a valid DBSS channel.
        dg = Datagram.create([doid2], 5, DBSS_ADD_POST_REMOVE)
        dg.add_uint32(0xF00B4511)
        dg.add_string(pr4.get_data())
        self.shard.send(dg)

        # Kill the DBSS and give the post-removes some time to be routed.
        self.dbss_daemon.stop()
        time.sleep(1.0)

        # We expect pr2 and pr3, the post-removes for doid2, to be routed to the shard channel.
        self.expectMany(self.shard, [pr2, pr3])

        # Followed by nothing else.
        self.expectNone(self.shard)

if __name__ == '__main__':
    unittest.main()
