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
    - type: database
      control: 777
      generate:
        min: 1000000
        max: 1001000
""" % test_dc

class TestDatabaseServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)

    @classmethod
    def tearDownClass(cls):
        cls.conn.close()
        cls.daemon.stop()

    def test_create_selectall(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(20))

        doids = []

        # Create a stored DistributedTestObject1 with no initial values...
        dg = Datagram.create([777], 20, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1) # Context
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.conn.send(dg)

        # The Database should return the context and do_id...
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([20], 777, DBSERVER_CREATE_STORED_OBJECT_RESP, remaining=4+4))
        self.assertTrue(dgi.read_uint32() == 1) # Check context
        doids[0] = dgi.read_uint32()
        self.assertTrue(doids[0] >= 1000000 and doids[0] <= 1001000) # do_id in valid range

        # Select all fields from the stored object
        dg = Datagram.create([777], 20, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(2) # Context
        dg.add_uint32(doids[0])
        self.conn.send(dg)

        # Retrieve object from the database, the default value for the required field should exist...
        dg = Datagram.create([20], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(setRequired1DefaultValue)
        dg.add_uint16(0) # Optional field count
        self.assertTrue(self.conn.expect(dg))

        # Create a stored DistributedTestObject3 missing a required value...
        dg = Datagram.create([777], 20, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(3) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(0) # Field count
        self.conn.send(dg)

        # Return should be invalid because it is missing a defaultless required field
        dg = Datagram.create([20], 777, DBSERVER_CREATE_STORED_OBJECT_RESP)
        dg.add_uint32(3) # Context
        dg.add_uint32(INVALID_DO_ID)
        self.assertTrue(self.conn.expect(dg))

        # Create a stored DistributedTestObject3 with an actual values...
        dg = Datagram.create([777], 20, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(4) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(91849)
        dg.add_uint16(setDb3)
        dg.add_string("You monster...")
        self.conn.send(dg)

        # The Database should return a new do_id...
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([20], 777, DBSERVER_CREATE_STORED_OBJECT_RESP, remaining=4+4))
        self.assertTrue(dgi.read_uint32() == 4) # Check context
        doids[1] = dgi.read_uint32()
        self.assertTrue(doids[1] >= 1000000 and doids[0] <= 1001000) # do_id in valid range
        self.assertTrue(doids[0] != doids[1]) # do_ids should be different

        # Retrieve object from the database...
        dg = Datagram.create([777], 20, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(5) # Context
        dg.add_uint32(doids[1])
        self.conn.send(dg)

        # Get values back from server
        dg = Datagram.create([20], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(5) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(setRequired1DefaultValue)
        dg.add_uint32(91849)
        dg.add_uint16(1) # Optional field count
        dg.add_uint16(setDb3)
        dg.add_string("You monster...")
        self.assertTrue(self.conn.expect(dg))

    def test_create_collisions(self):
        # TODO: Test for collisions after creating the max number of objects
        # TODO: Test that creating max+1 objects (w/o deletion) returns BAD_DO_ID
        # TODO: Test that used ids are reused after deleting some objects
        pass

    def test_ram(self):
        pass

if __name__ == '__main__':
    unittest.main()