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
    - type: dbss
      database: 200
      ranges:
          - 9000
          - 9999
""" % test_dc

class TestStateServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        shard = socket(AF_INET, SOCK_STREAM)
        shard.connect(('127.0.0.1', 57123))
        cls.shard = MDConnection(shard)
        cls.shard.send(Datagram.create_add_channel(5))

        database = socket(AF_INET, SOCK_STREAM)
        database.connect(('127.0.0.1', 57123))
        cls.database = MDConnection(database)
        cls.database.send(Datagram.create_add_channel(200))

    @classmethod
    def tearDownClass(cls):
        cls.database.send(Datagram.create_remove_channel(200))
        cls.database.close()
        cls.shard.send(Datagram.create_remove_channel(5))
        cls.shard.close()
        cls.daemon.stop()

    # Tests the messages OBJECT_SET_ZONE, OBJECT_DELETE_RAM
    def test_select_delete(self):
        self.database.flush()
        self.shard.flush()

        # Query all from an object which hasn't been loaded into ram
        dg = Datagram.create([9000], 5, STATESERVER_OBJECT_QUERY_ALL)
        dg.add_uint32(1) # Context
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9000, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9000) # object Id

        # Send back to the DBSS with some required values
        dg = Datagram.create([9000], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(32144123)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(23)
        self.database.send(dg)

        # Values should be returned with INVALID_LOCATION
        dg = Datagram.create(5, 9000, STATESERVER_OBJECT_QUERY_ALL_RESP)
        dg.add_uint32(INVALID_DO_ID) # Parent
        dg.add_uint32(INVALID_ZONE) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9000) # ID
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(23) # setRDbD5
        self.shard.expect(dg)

        # Destroy our object in ram...
        dg = Datagram.create([9000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9000) # Object Id
        self.shard.send(dg)

        # Object doesn't have a location and so shouldn't announce its disappearance...
        self.shard.expect_none()

        # Destroy our object...
        dg = Datagram.create([9000], 5, STATESERVER_OBJECT_DELETE_DISK)
        dg.add_uint32(9000) # Object Id
        self.shard.send(dg)

        # Database should expect a delete message
        dg = Datagram.create([200], 9000, DATABASE_OBJECT_DELETE)
        dg.add_uint32(9000) # Object Id
        self.database.expect(dg)

        # Object doesn't have a location and so shouldn't announcet its disappearance...
        self.shard.expect_none()

        # Get the object to ensure values are not cached-badly|stored-improperly by the DBSS
        dg = Datagram.create([9000], 5, STATESERVER_OBJECT_QUERY_ALL)
        dg.add_uint32(2) # Context
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9000, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9000) # object Id

        # Send back failure to DBSS
        dg = Datagram.create([9000], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Should recieve no stateserver object response
        self.shard.expect_none()

    # Tests that the DBSS is listening to the entire range it was configured with
    #def test_subscribe(self):
    #    self.fail("Test not implemented.")

if __name__ == '__main__':
    unittest.main()
