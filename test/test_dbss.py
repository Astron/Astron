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

CONTEXT_OFFSET = 1 + 8 + 8 + 2

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

    # Tests the messages OBJECT_QUERY_ALL
    def test_query_all(self):
        self.database.flush()
        self.shard.flush()

        ### Test for QueryAll while object exists in Database ###
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

        # Values should be returned from DBSS with INVALID_LOCATION
        dg = Datagram.create(5, 9000, STATESERVER_OBJECT_QUERY_ALL_RESP)
        dg.add_uint32(1) # Context
        dg.add_uint32(INVALID_DO_ID) # Parent
        dg.add_uint32(INVALID_ZONE) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9000) # ID
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(23) # setRDbD5
        dg.add_uint16(0) # Optional field count
        self.shard.expect(dg)



        ### Test for QueryAll while object DOES NOT exist in Database ###
        # Query all from an object which hasn't been loaded into ram
        dg = Datagram.create([9000], 5, STATESERVER_OBJECT_QUERY_ALL)
        dg.add_uint32(2) # Context
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9000, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9000) # object Id

        # This time pretend the object doesn't exist
        dg = Datagram.create([9000], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Should recieve no stateserver object response
        self.shard.expect_none()

    # Tests the message OBJECT_SET_ZONE
    def test_set_zone(self):
        self.database.flush()
        self.shard.flush()
        self.shard.send(Datagram.create_add_channel(80000<<32|100))
        self.shard.send(Datagram.create_add_channel(80000<<32|101))
        self.shard.send(Datagram.create_add_channel(80000<<32|102))

        ### Test for SetZone while object is not loaded into ram ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([9010], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(100) # Zone
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9010, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9010) # object Id

        # Send back to the DBSS with some required values
        dg = Datagram.create([9010], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(3117)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(97)
        self.database.send(dg)

        # See if it announces its entry into 100.
        dg = Datagram.create([80000<<32|100], 9010, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(100) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9000) # ID
        dg.add_uint32(3117) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(97) # setRDbD5
        self.shard.expect(dg)



        ### Test for SetZone while object is already loaded into ram
        ### (continues from above)
        # Move an object in ram to a different zone
        dg = Datagram.create([9010], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(101) # Zone
        self.shard.send(dg)

        # See if it announces its departure from 100...
        expected = []
        dg = Datagram.create([80000<<32|100], 5, STATESERVER_OBJECT_CHANGE_ZONE)
        dg.add_uint32(9010)
        dg.add_uint32(80000)
        dg.add_uint32(101)
        dg.add_uint32(80000)
        dg.add_uint32(100)
        expected.append(dg)
        # ...and its entry into 101.
        dg = Datagram.create([80000<<32|101], 9010, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(101) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9010) # ID
        dg.add_uint32(3117) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(97) # setRDbD5
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Remove object from ram
        dg = Datagram.create([9010], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9010)
        self.shard.send(dg)
        self.shard.flush()



        ### Test for multiple SetZone recieves while object is loading ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([9011], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(100) # Zone
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9011, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9011) # object Id

        # Send a couple more set_zones
        dg = Datagram.create([9011], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(101) # Zone
        self.shard.send(dg)
        dg = Datagram.create([9011], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(102) # Zone
        self.shard.send(dg)

        # Should not have recieved packets on either shard or database
        self.shard.expect_none()
        self.database.expect_none()

        # Send back to the DBSS with some required values
        dg = Datagram.create([9011], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(1)
        dg.add_uint16(setRDB3)
        dg.add_uint32(setRDB3DefaultValue)
        self.database.send(dg)

        # See if it announces its entry into 100.
        dg = Datagram.create([80000<<32|100], 9011, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(100) # Zone
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint32(9011) # ID
        dg.add_uint32(setRequired1DefaultValue)
        dg.add_uint32(setRDB3DefaultValue)
        self.shard.expect(dg)

        # See if it announces its departure from 100...
        expected = []
        dg = Datagram.create([80000<<32|100], 5, STATESERVER_OBJECT_CHANGE_ZONE)
        dg.add_uint32(9011)
        dg.add_uint32(80000)
        dg.add_uint32(101)
        dg.add_uint32(80000)
        dg.add_uint32(100)
        expected.append(dg)
        # ...and its entry into 101.
        dg = Datagram.create([80000<<32|101], 9011, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(101) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9011) # ID
        dg.add_uint32(setRequired1DefaultValue)
        dg.add_uint32(setRDB3DefaultValue)
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # See if it announces its departure from 101...
        expected = []
        dg = Datagram.create([80000<<32|101], 5, STATESERVER_OBJECT_CHANGE_ZONE)
        dg.add_uint32(9011)
        dg.add_uint32(80000)
        dg.add_uint32(102)
        dg.add_uint32(80000)
        dg.add_uint32(101)
        expected.append(dg)
        # ...and its entry into 102.
        dg = Datagram.create([80000<<32|102], 9011, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(80000) # Parent
        dg.add_uint32(102) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9011) # ID
        dg.add_uint32(setRequired1DefaultValue)
        dg.add_uint32(setRDB3DefaultValue)
        expected.append(dg)
        self.assertTrue(self.c.expect_multi(expected, only=True))

        # Remove object from ram
        dg = Datagram.create([9011], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9011)
        self.shard.send(dg)
        self.shard.flush()


        ### Clean up ###
        self.shard.send(Datagram.create_remove_channel(80000<<32|100))
        self.shard.send(Datagram.create_remove_channel(80000<<32|101))
        self.shard.send(Datagram.create_remove_channel(80000<<32|102))

    # Tests the messages OBJECT_DELETE_DISK, OBJECT_DELETE_RAM
    def test_delete(self):
        self.database.flush()
        self.shard.flush()
        self.shard.send(Datagram.create_add_channel(90000<<32|200))

        ### Test for DelDisk ###
        # Destroy our object...
        dg = Datagram.create([9020], 5, STATESERVER_OBJECT_DELETE_DISK)
        dg.add_uint32(9020) # Object Id
        self.shard.send(dg)

        # Object doesn't have a location and so shouldn't announcet its disappearance...
        self.shard.expect_none()

        # Database should expect a delete message
        dg = Datagram.create([200], 9000, DATABASE_OBJECT_DELETE)
        dg.add_uint32(9020) # Object Id
        self.database.expect(dg)



        ### Test for SetZone->DelRam ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([9021], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(200) # Zone
        self.shard.send(dg)

        # Give the DBSS values from the database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9021, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9021) # object Id
        dg = Datagram.create([9021], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(setRDB3DefaultValue)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(setRDbD5DefaultValue)
        self.database.send(dg)

        # Ignore entry notification, we're not testing set_zone right now
        self.shard.flush()

        # Destroy our object in ram...
        dg = Datagram.create([9021], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9021) # Object Id
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<32|200], 9021, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9021)
        self.assertTrue(self.shard.expect(dg))



        ### Test for SetZone->DelRam->DelDisk ### (continues from last)
        # Destroy our object on disk...
        dg = Datagram.create([9021], 5, STATESERVER_OBJECT_DELETE_DISK)
        dg.add_uint32(9021)
        self.shard.send(dg)

        # Object no longer has a location and so shouldn't announce its disappearance...
        self.shard.expect_none()

        # Database should expect a delete message
        dg = Datagram.create([200], 9021, DATABASE_OBJECT_DELETE)
        dg.add_uint32(9021) # Object Id
        self.database.expect(dg)



        ### Test for SetZone->DelDisk ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([9022], 5, STATESERVER_OBJECT_SET_ZONE)
        dg.add_uint32(90000) # Parent
        dg.add_uint32(200) # Zone
        self.shard.send(dg)

        # Give the DBSS values from the database
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9022, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9022) # object Id
        dg = Datagram.create([9022], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(333444)
        dg.add_uint16(setFoo)
        dg.add_uint16(3344)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(34)
        self.database.send(dg)

        # Destroy our object on disk...
        dg = Datagram.create([9022], 5, STATESERVER_OBJECT_DELETE_DISK)
        dg.add_uint32(9022)
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<32|200], 9022, STATESERVER_OBJECT_DELETE_DISK)
        dg.add_uint32(9022)
        self.assertTrue(self.shard.expect(dg))

        # Database should expect a delete message
        dg = Datagram.create([200], 9022, DATABASE_OBJECT_DELETE)
        dg.add_uint32(9022) # Object Id
        self.database.expect(dg)

        # Check that Ram/Requried fields still exist in ram still
        dg = Datagram.create([9022], 5, STATESERVER_OBJECT_QUERY_ALL)
        dg.add_uint32(1) # Context
        self.shard.send(dg)

        # Reply to database_get_all with object does not exist (was deleted)
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9022, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9022) # object Id
        dg = Datagram.create([9022], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect ram/required fields to be returned (with valid location)
        dg = Datagram.create(5, 9022, STATESERVER_OBJECT_QUERY_ALL_RESP)
        dg.add_uint32(1) # Context
        dg.add_uint32(90000) # Parent
        dg.add_uint32(200) # Zone
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint32(9022) # ID
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(23) # setRDbD5
        dg.add_uint16(0) # Optional field count
        self.shard.expect(dg)



        ### Test for SetZone->DelDisk->DelRam ### (continues from last)
        # Destroy our object on ram...
        dg = Datagram.create([9022], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9022)
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<32|200], 9022, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(9022)
        self.assertTrue(self.shard.expect(dg))

        # Check that object no longer exists
        dg = Datagram.create([9022], 5, STATESERVER_OBJECT_QUERY_ALL)
        dg.add_uint32(2) # Context
        self.shard.send(dg)

        # Reply to database_get_all with object does not exist (was deleted)
        dg = self.database.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([200], 9022, DATABASE_OBJECT_GET_ALL, 4+4))
        context = dgi.read_uint32() # Get context
        self.assertTrue(dgi.read_uint32() == 9022) # object Id
        dg = Datagram.create([9022], 200, DATABASE_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect no response
        self.shard.expect_none()


        ### Clean Up ###
        self.shard.send(Datagram.create_remove_channel(90000<<32|200))

    # Tests that the DBSS is listening to the entire range it was configured with
    def test_subscribe(self):
        self.database.flush()
        self.shard.flush()

        probe_context = 0
        def probe(doid):
            # Try a query all on the id
            dg = Datagram.create([doid], 5, STATESERVER_OBJECT_QUERY_ALL)
            dg.add_uint32(++probe_context) # Context
            self.shard.send(dg)

            # Check if recieved database query
            dg = self.database.recv_maybe()
            if dg is None:
                return False

            # Cleanup message
            dgi = DatagramIterator(dg)
            dgi.seek(CONTEXT_OFFSET)
            context = dgi.read_uint32() # Get context
            dg = Datagram.create([doid], 200, DATABASE_OBJECT_GET_ALL_RESP)
            dg.add_uint32(context)
            dg.add_uint8(SUCCESS)
            dg.add_uint16(DistributedTestObject3)
            dg.add_uint16(1)
            dg.add_uint16(setRDB3)
            dg.add_uint32(setRDB3DefaultValue)
            self.database.send(dg)
            self.shard.flush()

        self.assertFalse(probe(900))
        self.assertFalse(probe(999))
        self.assertFalse(probe(8400))
        self.assertFalse(probe(8980))
        self.assertFalse(probe(8999))
        self.assertTrue(probe(9000))
        self.assertTrue(probe(9001))
        self.assertTrue(probe(9047))
        self.assertTrue(probe(9236))
        self.assertTrue(probe(9500))
        self.assertTrue(probe(9856))
        self.assertTrue(probe(9999))
        self.assertFalse(probe(10000))
        self.assertFalse(probe(10017))
        self.assertFalse(probe(14545))
        self.assertFalse(probe(90000))
        self.assertFalse(probe(99000))
        self.assertFalse(probe(99990))

    # Tests the message STATESERVER_OBJECT_UPDATE_FIELD, STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE
    def test_update_db(self):
        self.shard.flush()
        self.database.flush()

        ### Test for UpdateField with db field on unloaded object###
        # Update field on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(9030) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        self.shard.send(dg)

        # Expect database field to be sent to database
        dg = Datagram.create([200], 9030, DATABASE_OBJECT_SET_FIELD)
        dg.add_uint32(9030) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        self.database.expect(dg)


        ### Test for UpdateFieldMultiple with all db fields on unloaded object ###
        # Update field multiple on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE)
        dg.add_uint32(9030) # id
        dg.add_uint16(2) # field count
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        dg.add_uint16(setRDB)
        dg.add_uint32(8192)
        self.shard.send(dg)

        # Expect database fields to be sent to database
        dg = Datagram.create([200], 9030, DATABASE_OBJECT_SET_FIELDS)
        dg.add_uint32(9030) # id
        dg.add_uint16(2) # field count
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        dg.add_uint16(setRDB)
        dg.add_uint32(8192)
        self.shard.send(dg)


        ### Test for UpdateField with non-db field on unloaded object ###
        # Update field on stateserver object
        dg = Datagram.create([200], 9030, DATABASE_OBJECT_SET_FIELD)
        dg.add_uint32(9030) # id
        dg.add_uint16(setRequired1)
        dg.add_uint16(512)
        self.database.expect(dg)

        # Expect none at database
        self.database.expect_none()

        ### Test for UpdateField with all non-db fields on unloaded object ###
        # Update fields on stateserver object
        dg = Datagram.create([200], 9030, DATABASE_OBJECT_SET_FIELD)
        dg.add_uint32(9030) # id
        dg.add_uint16(setRequired1)
        dg.add_uint16(512)
        dg.add_uint16(setBR1)
        dg.add_string("Sleeping in the middle of a summer afternoon.")
        self.database.expect(dg)

        # Expect none at database
        self.database.expect_none()

if __name__ == '__main__':
    unittest.main()
