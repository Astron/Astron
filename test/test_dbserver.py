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
        max: 1000010
      engine:
        type: filesystem
        foldername: objss
""" % test_dc
CREATE_DOID_OFFSET = 1 + 8 + 8 + 2 + 4
VERIFY_DELETE_OBJECT = 0x44696521
VERIFY_DELETE_QUERY = 0x4b696c6c

class DatabaseBaseTests(object):
    @classmethod
    def tearDownClass(cls):
        cls.conn.close()
        cls.daemon.stop()

    def createEmptyObjGetId(self, sender, context):
        # Create an object
        dg = Datagram.create([777], sender, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1) # Context
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.conn.send(dg)

        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        dgi.seek(CREATE_DOID_OFFSET)
        return dgi.read_uint32()

    def deleteObject(self, sender, doid):
        dg = Datagram.create([777], sender, DBSERVER_DELETE_STORED_OBJECT)
        dg.add_uint32(VERIFY_DELETE_OBJECT)
        dg.add_uint32(doid)
        self.conn.send(dg)

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
        doids.append(dgi.read_uint32())
        self.assertTrue(doids[0] >= 1000000 and doids[0] <= 1000010) # do_id in valid range

        # Select all fields from the stored object
        dg = Datagram.create([777], 20, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(2) # Context
        dg.add_uint32(doids[0])
        self.conn.send(dg)

        # Retrieve object from the database, we stored no DB values, so get none back
        dg = Datagram.create([20], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(FOUND)
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.assertTrue(self.conn.expect(dg)) # Expecting SELECT_RESP with no values

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
        self.assertTrue(self.conn.expect(dg)) # Expecting CREATE_RESP with BAD_DO_ID

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
        doids.append(dgi.read_uint32())
        self.assertTrue(doids[1] >= 1000000 and doids[0] <= 1000010) # do_id in valid range
        self.assertTrue(doids[0] != doids[1]) # do_ids should be different

        # Retrieve object from the database...
        dg = Datagram.create([777], 20, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(5) # Context
        dg.add_uint32(doids[1])
        self.conn.send(dg)

        # Get values back from server
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([20], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP))
        self.assertTrue(dgi.read_uint32() == 5) # Check context
        self.assertTrue(dgi.read_uint8() == FOUND)
        self.assertTrue(dgi.read_uint16() == DistributedTestObject3)
        self.assertTrue(dgi.read_uint16() == 2) # Check field count
        for x in xrange(2):
            field = dgi.read_uint16()
            if field == setRDB3:
                self.assertTrue(dgi.read_uint32() == 91849)
            elif field == setDb3:
                self.assertTrue(dgi.read_string() == "You monster...")
            else:
                self.fail("Bad field type")

        # Try selecting an ID that doesn't exist
        dg = Datagram.create([777], 20, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(6) # Context
        dg.add_uint32(78787) # Non-existant ID
        self.conn.send(dg)

        # Get failure from server
        dg = Datagram.create([20], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint8(NOT_FOUND)
        self.assertTrue(self.conn.expect(dg))

        # Cleanup
        for doid in doids:
            self.deleteObject(20, doid)
        self.conn.send(Datagram.create_remove_channel(20))

    def test_delete(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(30))

        # Create an object, get its doid
        doid = self.createEmptyObjGetId(30, 1)

        # Delete the object
        self.deleteObject(30, doid)

        # Check to make sure the object is deleted
        dg = Datagram.create([777], 30, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(2) # Context
        dg.add_uint32(doid)
        self.conn.send(dg)

        # Get failure from database
        dg = Datagram.create([30], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(NOT_FOUND)
        self.assertTrue(self.conn.expect(dg)) # object deleted

        # Create some other objects
        doidA = self.createEmptyObjGetId(30, 3)
        doidB = self.createEmptyObjGetId(30, 4)

        # Delete object "A"
        self.deleteObject(30, doidA)

        # Check to make sure object "B" isn't affected
        dg = Datagram.create([777], 30, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(5) # Context
        dg.add_uint32(doidB)
        self.conn.send(dg)

        # Reponse for object "B"
        dg = Datagram.create([30], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(5) # Context
        dg.add_uint8(FOUND)
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.assertTrue(self.conn.expect(dg)) # object "B" not deleted

        # Try to delete object "B" with a bad verify_code
        dg = Datagram.create([30], 777, DBSERVER_DELETE_STORED_OBJECT)
        dg.add_uint32(1337)
        dg.add_uint32(doidB)
        self.conn.send(dg)

        # Check to make sure object "B" isn't affected
        dg = Datagram.create([777], 30, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(6) # Context
        dg.add_uint32(doidB)
        self.conn.send(dg)

        # Reponse for object "B"
        dg = Datagram.create([30], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint8(FOUND)
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.assertTrue(self.conn.expect(dg)) # object "B" not deleted

        # Cleanup
        self.deleteObject(doidB)
        self.conn.send(Datagram.create_remove_channel(30))

    def test_create_collisions(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(40))

        doids = []

        # Create the maximum number of objects we can assign
        doid = self.createEmptyObjGetId(40, len(doids))
        while doid != 0 and len(doids) < 15:
            doids.append(doid)
            doid = self.createEmptyObjGetId(40, len(doids))

        self.assertTrue(len(set(doids)) == len(doids)) # Check if duplicate do_ids exist
        self.assertTrue(len(doids) == 11) # Check we recieved the max do_ids we requested
        self.assertTrue(doid == INVALID_DO_ID) # Check the last object returned was BAD_DO_ID (0x0)

        # Delete an object
        self.deleteObject(40, doids[6])

        # Get new object with the last remaining id
        newdoid = self.createEmptyObjGetId(40, 16)
        self.assertTrue(newdoid == doids[6])

        # Delete multiple objects
        self.deleteObject(40, doids[0])
        self.deleteObject(40, doids[1])
        self.deleteObject(40, doids[2])
        doids = doids[3:]

        # Create an object, it shouldn't collide
        doid = self.createEmptyObjGetId(40, 17)
        for do in doids:
            self.assertTrue(do != doid)

        # Cleanup
        self.deleteObject(40, doids[0])
        for do in doids:
            self.deleteObject(40, do)
        self.conn.send(Datagram.create_remove_channel(40))

    def test_ram(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(50))

        # Create a stored DistributedTestObject3 with actual values and non-db/ram values we don't care about...
        dg = Datagram.create([777], 50, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(5) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(91849)
        dg.add_uint16(setBA1)
        dg.add_uint16(239)
        dg.add_uint16(setDb3)
        dg.add_string("You monster...")
        dg.add_uint16(setB1)
        dg.add_uint8(17)
        dg.add_uint16(setBR1)
        dg.add_string("Fiddlesticks!!!")
        self.conn.send(dg)

        # The Database should return a new do_id...
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([50], 777, DBSERVER_CREATE_STORED_OBJECT_RESP))
        self.assertTrue(dgi.read_uint32() == 1) # Check context
        doid = dgi.read_uint32()

        # Retrieve object from the database...
        dg = Datagram.create([777], 50, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(2) # Context
        dg.add_uint32(doid)
        self.conn.send(dg)

        # Get values back from server
        dg = Datagram.create([50], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(FOUND)
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(91849)
        dg.add_uint16(setDb3)
        dg.add_string("You monster...")
        self.assertTrue(self.conn.expect(dg))

        # Cleanup
        self.deleteObject(50, doid)
        self.conn.send(Datagram.create_remove_channel(50))

class TestDatabaseServer(unittest.TestCase, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)


if __name__ == '__main__':
    unittest.main()