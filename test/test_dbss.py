#!/usr/bin/env python2
import unittest
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
    - type: dbss
      database: 1200
      ranges:
          - min: 9000
            max: 9999
""" % (USE_THREADING, test_dc)

CONTEXT_OFFSET = 1 + (CHANNEL_SIZE_BYTES*2) + 2

def appendMeta(datagram, doid=None, parent=None, zone=None, dclass=None):
    if doid is not None:
        datagram.add_doid(doid)
    if parent is not None:
        datagram.add_doid(parent)
    if zone is not None:
        datagram.add_zone(zone)
    if dclass is not None:
        datagram.add_uint16(dclass)

class TestDBStateServer(ProtocolTest):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

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
        cls.daemon.stop()

   # Tests the message OBJECT_ACTIVATE_WITH_DEFAULTS{_OTHER,}
    def test_activate(self):
        self.database.flush()
        self.shard.flush()
        self.shard.send(Datagram.create_add_channel(80000<<ZONE_SIZE_BITS|100))
        self.shard.send(Datagram.create_add_channel(80000<<ZONE_SIZE_BITS|101))

        doid1 = 9001
        doid2 = 9002

        ### Test for Activate while object is not loaded into ram ###
        # Enter an object into ram from the disk by Activating it
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid1, 80000, 100)
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
        dg.add_uint16(setRDB3)
        dg.add_uint32(3117)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(97)
        self.database.send(dg)

        # See if it announces its entry into 100.
        dg = Datagram.create([80000<<ZONE_SIZE_BITS|100], doid1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, 80000, 100, DistributedTestObject5)
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(3117) # setRDB3
        self.expect(self.shard, dg)


        ### Test for Activate while object is already loaded into ram
        ### (continues from above)
        # Try to reactivate an active object
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid1, 80000, 101)
        self.shard.send(dg)

        # The object is already activated, so this should be ignored
        self.expectNone(self.shard)
        self.expectNone(self.database)

        # Remove object from ram
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid1)
        self.shard.send(dg)
        self.shard.flush()


        ### Test for Activate on Database object with other fields ###
        # Enter an object into ram from the disk by Activating it; with
        # overrides
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER)
        appendMeta(dg, doid1, 80000, 100)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2) # Two other fields:
        dg.add_uint16(setRequired1)
        dg.add_uint32(0x00a49de2)
        dg.add_uint16(setBR1)
        dg.add_string('V ybir Whar')
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
        dg.add_uint32(0x00a49de2) # setRequired1
        dg.add_uint32(3117) # setRDB3
        dg.add_uint16(1) # One other field:
        dg.add_uint16(setBR1)
        dg.add_string('V ybir Whar')
        self.expect(self.shard, dg)

        # Remove object from ram
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid1)
        self.shard.send(dg)
        self.shard.flush()


        ### Test for Activate on non-existent Database object ###
        # Enter an object into ram with activate
        dg = Datagram.create([doid2], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid2, 80000, 100)
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid2, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid2) # object Id

        # Send back to the DBSS with failed-to-find object
        dg = Datagram.create([doid2], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect no entry messages
        self.expectNone(self.shard)

        ### Clean up ###
        self.shard.send(Datagram.create_remove_channel(80000<<ZONE_SIZE_BITS|100))
        self.shard.send(Datagram.create_remove_channel(80000<<ZONE_SIZE_BITS|101))

    # Tests the messages OBJECT_GET_ALL
    def test_get_all(self):
        self.database.flush()
        self.shard.flush()

        doid1 = 9011

        ### Test for GetAll while object exists in Database ###
        # Query all from an object which hasn't been loaded into ram
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(1) # Context
        dg.add_doid(doid1) # Id
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
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(32144123)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(23)
        self.database.send(dg)

        # Values should be returned from DBSS with INVALID_LOCATION
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(1) # Context
        appendMeta(dg, doid1, INVALID_DO_ID, INVALID_ZONE, DistributedTestObject5)
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(23) # setRDbD5
        dg.add_uint16(0) # Optional field count
        self.expect(self.shard, dg)



        ### Test for GetAll while object DOES NOT exist in Database ###
        # Query all from an object which hasn't been loaded into ram
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(2) # Context
        dg.add_doid(doid1) # Id
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid1) # object Id

        # This time pretend the object doesn't exist
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Should receive no stateserver object response
        self.expectNone(self.shard)



        ### Test for caching of GetAll for Activate messages ###
        self.shard.send(Datagram.create_add_channel(33000<<ZONE_SIZE_BITS|33))
        # Get all from an object
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(3) # Context
        dg.add_doid(doid1) # Id
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid1) # object Id

        # Try to activate the object
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid1, 33000, 33)
        self.shard.send(dg)

        # Expect not to receive another request at the database
        self.expectNone(self.database)

        # Send back to the DBSS with some required values
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(32144123)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(23)
        self.database.send(dg)

        expected = []
        # Expect to receive a reply to the original message ...
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(3) # Context
        appendMeta(dg, doid1, INVALID_DO_ID, INVALID_ZONE, DistributedTestObject5)
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        dg.add_uint8(23) # setRDbD5
        dg.add_uint16(0) # Optional field count
        expected.append(dg)
        # As well as the object's entry into the location
        dg = Datagram.create([33000<<ZONE_SIZE_BITS|33], doid1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        appendMeta(dg, doid1, 33000, 33, DistributedTestObject5)
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(32144123) # setRDB3
        expected.append(dg)
        self.expectMany(self.shard, expected)


        ### Cleanup ###
        self.shard.send(Datagram.create_remove_channel(33000<<ZONE_SIZE_BITS|33))

    # Tests the messages OBJECT_DELETE_DISK, OBJECT_DELETE_RAM
    def test_delete(self):
        return
        self.database.flush()
        self.shard.flush()
        self.shard.send(Datagram.create_add_channel(90000<<ZONE_SIZE_BITS|200))

        doid1 = 9021
        doid2 = 9022
        doid3 = 9023

        ### Test for DelDisk ###
        # Destroy our object...
        dg = Datagram.create([doid1], 5, DBSS_OBJECT_DELETE_DISK)
        dg.add_doid(doid1) # Object Id
        self.shard.send(dg)

        # Object doesn't have a location and so shouldn't announce its disappearance...
        self.expectNone(self.shard)

        # Database should expect a delete message
        dg = Datagram.create([1200], doid1, DBSERVER_OBJECT_DELETE)
        dg.add_doid(doid1) # Object Id
        self.expect(self.database, dg)



        ### Test for Activate->DelRam ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([doid2], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid2, 90000, 200)
        self.shard.send(dg)

        # The dbss asks for values for the object from the database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid2, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid2) # Id

        # Tell it the object exists
        dg = Datagram.create([doid2], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(0)
        self.database.send(dg)

        # Ignore entry notification, we're not testing that right now
        self.shard.flush()

        # Destroy our object in ram...
        dg = Datagram.create([doid2], 13, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid2) # Object Id
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<ZONE_SIZE_BITS|200], 13, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid2)
        self.expect(self.shard, dg)



        ### Test for (Activate->DelRam)->DelDisk ### (continues from last)
        # Destroy our object on disk...
        dg = Datagram.create([doid2], 5, DBSS_OBJECT_DELETE_DISK)
        dg.add_doid(doid2)
        self.shard.send(dg)

        # Object no longer has a location and so shouldn't announce its disappearance...
        self.expectNone(self.shard)

        # Database should expect a delete message
        dg = Datagram.create([1200], doid2, DBSERVER_OBJECT_DELETE)
        dg.add_doid(doid2) # Object Id
        self.expect(self.database, dg)



        ### Test for Activate->DelDisk ###
        # Enter an object into ram from the disk by setting its zone
        dg = Datagram.create([doid3], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        appendMeta(dg, doid3, 90000, 200)
        self.shard.send(dg)

        # Give the DBSS values from the database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid3, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid3) # object Id
        dg = Datagram.create([doid3], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
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

        # Ignore entry notification
        self.shard.flush()

        # Destroy our object on disk...
        dg = Datagram.create([doid3], 13, DBSS_OBJECT_DELETE_DISK)
        dg.add_doid(doid3)
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<ZONE_SIZE_BITS|200], 13, DBSS_OBJECT_DELETE_DISK)
        dg.add_doid(doid3)
        self.expect(self.shard, dg)

        # Database should expect a delete message
        dg = Datagram.create([1200], doid3, DBSERVER_OBJECT_DELETE)
        dg.add_doid(doid3) # Object Id
        self.expect(self.database, dg)

        # Check that Ram/Requried fields still exist in ram still
        dg = Datagram.create([doid3], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(1) # Context
        self.shard.send(dg)

        # DBSS should request the value of foo from the DBSS
        msgtype, context = None, None
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None) # Expecting DB_GET_FIELD(S)
        dgi = DatagramIterator(dg)
        # Will expect either GET_FIELD...
        if dgi.matches_header([1200], doid3, DBSERVER_OBJECT_GET_FIELD, 4+2)[0]:
            msgtype = DBSERVER_OBJECT_GET_FIELD
            context = dgi.read_uint32()
            self.assertEquals(dgi.read_doid(), doid3)
            self.assertEquals(dgi.read_uint16(), setFoo)
        # ... or GET_FIELDS with 1 field, both satisify the protocol
        elif dgi.matches_header([1200], doid3, DBSERVER_OBJECT_GET_FIELDS, 4+2+2)[0]:
            msgtype = DBSERVER_OBJECT_GET_FIELDS
            context = dgi.read_uint32()
            self.assertEquals(dgi.read_doid(), doid3)
            self.assertEquals(dgi.read_uint16(), 1) # Field count
            self.assertEquals(dgi.read_uint16(), setFoo)
        else:
            self.fail("Unexpected message type.")

        # Return Failure to DBSS
        if msgtype is DBSERVER_OBJECT_GET_FIELD:
            dg = Datagram.create([doid3], 1200, DBSERVER_OBJECT_GET_FIELD_RESP)
            dg.add_uint32(context)
            dg.add_uint8(FAILURE)
        else:
            dg = Datagram.create([doid3], 1200, DBSERVER_OBJECT_GET_FIELDS_RESP)
            dg.add_uint32(context)
            dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect ram/required fields to be returned (with valid location)
        dg = Datagram.create([5], 9022, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(1) # Context
        appendMeta(doid3, 90000, 200, DistributedTestObject5)
        dg.add_uint32(setRequired1DefaultValue) # setRequired1
        dg.add_uint32(333444) # setRDB3
        dg.add_uint8(34) # setRDbD5
        dg.add_uint16(0) # Optional field count
        self.expect(self.shard, dg)



        ### Test for (Activate->DelDisk)->DelRam ### (continues from last)
        # Destroy our object on ram...
        dg = Datagram.create([doid3], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid3)
        self.shard.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([90000<<ZONE_SIZE_BITS|200], doid3, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid3)
        self.expect(self.shard, dg)

        # Check that object no longer exists
        dg = Datagram.create([doid3], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(2) # Context
        self.shard.send(dg)

        # Reply to database_get_all with object does not exist (was deleted)
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid3, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid3) # object Id
        dg = Datagram.create([doid3], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect no response
        self.expectNone(self.shard)


        ### Clean Up ###
        self.shard.send(Datagram.create_remove_channel(90000<<ZONE_SIZE_BITS|200))

    # Tests that the DBSS is listening to the entire range it was configured with
    def test_subscribe(self):
        self.database.flush()
        self.shard.flush()

        self.probe_context = 0
        def probe(doid):
            self.probe_context += 1

            # Try a query all on the id
            dg = Datagram.create([doid], 5, STATESERVER_OBJECT_GET_ALL)
            dg.add_uint32(self.probe_context) # Context
            dg.add_doid(doid)
            self.shard.send(dg)

            # Check if received database query
            dg = self.database.recv_maybe()
            if dg is None:
                return False

            # Cleanup message
            dgi = DatagramIterator(dg)
            dgi.seek(CONTEXT_OFFSET)
            context = dgi.read_uint32() # Get context
            dg = Datagram.create([doid], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
            dg.add_uint32(context)
            dg.add_uint8(SUCCESS)
            dg.add_uint16(DistributedTestObject3)
            dg.add_uint16(1)
            dg.add_uint16(setRDB3)
            dg.add_uint32(1200)
            self.database.send(dg)
            self.shard.flush()
            return True

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

    # Tests the message STATESERVER_OBJECT_SET_FIELD, STATESERVER_OBJECT_SET_FIELDS
    def test_set(self):
        self.shard.flush()
        self.database.flush()
        self.shard.send(Datagram.create_add_channel(70000<<ZONE_SIZE_BITS|300))

        ### Test for SetField with db field on unloaded object###
        # Update field on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(9030) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        self.shard.send(dg)

        # Expect database field to be sent to database
        dg = Datagram.create([1200], 9030, DBSERVER_OBJECT_SET_FIELD)
        dg.add_doid(9030) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        self.expect(self.database, dg)



        ### Test for SetFields with all db fields on unloaded object ###
        # Update field multiple on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9030) # id
        dg.add_uint16(2) # field count
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        dg.add_uint16(setRDB3)
        dg.add_uint32(8192)
        self.shard.send(dg)

        # Expect database fields to be sent to database
        dg = Datagram.create([1200], 9030, DBSERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9030) # Id
        dg.add_uint16(2) # Field count: 2
        dg.add_uint16(setRDB3)
        dg.add_uint32(8192)
        dg.add_uint16(setFoo)
        dg.add_uint16(4096)
        self.expect(self.database, dg)


        ### Test for SetField with non-db field on unloaded object ###
        # Update field on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(9030) # id
        dg.add_uint16(setRequired1)
        dg.add_uint16(512)
        self.shard.send(dg)

        # Expect none at database
        self.expectNone(self.database)

        ### Test for SetFields with all non-db fields on unloaded object ###
        # Update fields on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9030) # id
        dg.add_uint16(setRequired1)
        dg.add_uint32(313131)
        dg.add_uint16(setBR1)
        dg.add_string("Sleeping in the middle of a summer afternoon.")
        self.shard.send(dg)

        # Expect none at database
        self.expectNone(self.database)

        ### Test for SetFields with mixed db and non-db fields on unloaded object ###
        # Update field multiple on stateserver object
        dg = Datagram.create([9030], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9030) # id
        dg.add_uint16(3) # field count
        dg.add_uint16(setFoo)
        dg.add_uint16(1337)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xBADCAFE0)
        dg.add_uint16(setRDB3)
        dg.add_uint32(4080)
        self.shard.send(dg)

        # Expect database fields to be sent to database
        dg = Datagram.create([1200], 9030, DBSERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9030) # Id
        dg.add_uint16(2) # Field count: 2
        dg.add_uint16(setRDB3)
        dg.add_uint32(4080)
        dg.add_uint16(setFoo)
        dg.add_uint16(1337)
        self.expect(self.database, dg)

        ### Test for SetField with db field on loaded object ###
        # Activate object with defaults
        dg = Datagram.create([9031], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        dg.add_doid(9031) # Id
        dg.add_doid(70000) # Parent
        dg.add_zone(300) # Zone
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], 9031, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), 9031) # object Id

        # Send back to the DBSS with some required values
        dg = Datagram.create([9031], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(777)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(222)
        self.database.send(dg)

        # Ignore entry into 300, we're not testing that here
        self.shard.flush()

        # Send UpdateField with db field
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(9031) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(6604)
        self.shard.send(dg)

        # Expect database field to be sent to database
        dg = Datagram.create([1200], 9031, DBSERVER_OBJECT_SET_FIELD)
        dg.add_doid(9031) # id
        dg.add_uint16(setFoo)
        dg.add_uint16(6604)
        self.expect(self.database, dg)



        ### Test for SetFields with all db fields on loaded object
        ### (continues from previous)
        # Update field multiple on stateserver object
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9031) # id
        dg.add_uint16(2) # field count
        dg.add_uint16(setFoo)
        dg.add_uint16(7722)
        dg.add_uint16(setRDB3)
        dg.add_uint32(18811881)
        self.shard.send(dg)

        # Expect database fields to be sent to database
        dg = Datagram.create([1200], 9031, DBSERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9031) # Id
        dg.add_uint16(2) # Field count: 2
        dg.add_uint16(setRDB3)
        dg.add_uint32(18811881)
        dg.add_uint16(setFoo)
        dg.add_uint16(7722)
        self.expect(self.database, dg)

        ### Test for SetField with non-db field on loaded object
        ### (continues from previous)
        # Update field on stateserver object
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(9031) # id
        dg.add_uint16(setRequired1)
        dg.add_uint32(512)
        self.shard.send(dg)

        # Expect none at database
        self.expectNone(self.database)

        # Ignore any previous setField broadcasts
        self.shard.flush()

        # Get the values back to check if they're updated
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(1) # Context
        dg.add_doid(9031) # Id
        self.shard.send(dg)

        # Expect none at database
        self.expectNone(self.database)

        # Updated values should be returned from DBSS
        dg = Datagram.create([5], 9031, STATESERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(1) # Context
        appendMeta(dg, 9031, 70000, 300, DistributedTestObject5)
        dg.add_uint32(512) # setRequired1
        dg.add_uint32(18811881) #setRDB3
        dg.add_uint8(222) # setRDbD5
        dg.add_uint16(0) # Optional fields count
        self.expect(self.shard, dg)



        ### Test for SetFields with all non-db fields on loaded object
        ### (continues from previous)
        # Update fields on stateserver object
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(9031) # id
        dg.add_uint16(2) # Field count: 2
        dg.add_uint16(setRequired1)
        dg.add_uint32(393939)
        dg.add_uint16(setBR1)
        dg.add_string("Sleeping in the middle of a summer afternoon.")
        self.shard.send(dg)

        # Expect none at database
        self.expectNone(self.database)

        # Ignore broadcast
        self.shard.flush()

        # Get the values back to check if they're updated
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_GET_ALL)
        dg.add_uint32(2) # Context
        dg.add_doid(9031)
        self.shard.send(dg)

        # DBSS should expect none
        self.expectNone(self.database)

        # Updated values should be returned from DBSS
        dg = self.shard.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([5], 9031, STATESERVER_OBJECT_GET_ALL_RESP))
        self.assertEquals(dgi.read_uint32(), 2) # Context
        self.assertEquals(dgi.read_doid(), 9031) # ID
        self.assertEquals(dgi.read_doid(), 70000) # Parent
        self.assertEquals(dgi.read_zone(), 300) # Zone
        self.assertEquals(dgi.read_uint16(), DistributedTestObject5)
        self.assertEquals(dgi.read_uint32(), 393939) # setRequired1
        self.assertEquals(dgi.read_uint32(), 18811881) # setRDB3
        self.assertEquals(dgi.read_uint8(), 222) # setRDbD5
        self.assertEquals(dgi.read_uint16(), 1) # Optional field count
        self.assertEquals(dgi.read_uint16(), setBR1)
        self.assertEquals(dgi.read_string(), "Sleeping in the middle of a summer afternoon.")

        # Remove object from ram after tests
        dg = Datagram.create([9031], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(9031) # ID
        self.shard.send(dg)

        # Ignore propagated delete_ram messages
        self.shard.flush()

        ### Cleanup ###.
        self.shard.send(Datagram.create_remove_channel(70000<<ZONE_SIZE_BITS|300))

    def test_get_fields(self):
        self.shard.flush()
        self.database.flush()

        doid1 = 9040

        ### Test for GetField with ram db field on inactive object###
        # Query field from StateServer object
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(1) # Context
        dg.add_doid(doid1) # ID
        dg.add_uint16(setDb3)
        self.shard.send(dg)

        # Expect database query
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None) # Expecting DBGetField
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_FIELD))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), doid1)
        self.assertEquals(dgi.read_uint16(), setDb3)

        # Return field value to DBSS
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setDb3)
        dg.add_string("Slam-bam-in-a-can!")
        self.database.send(dg)

        # Expect field sent back to caller
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(1) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(setDb3)
        dg.add_string("Slam-bam-in-a-can!")
        self.expect(self.shard, dg) # Expecting GetField success


        ### Test for GetField with existing non-ram db field ###
        # Query field from StateServer object
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_FIELD)
        dg.add_uint32(2) # Context
        dg.add_doid(doid1) # ID
        dg.add_uint16(setFoo)
        self.shard.send(dg)

        # Expect nothing at database, DBSS only returns ram/required db fields
        self.expectNone(self.database)

        # Expect failure resp sent to caller
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_FIELD_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(FAILURE)
        self.expect(self.shard, dg) # Expecting GetField failure



        ### Test for GetFields with ram fields on inactive object ###
        # Query field from StateServer object
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(3) # Context
        dg.add_doid(doid1) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setDb3)
        dg.add_uint16(setRDB3)
        self.shard.send(dg)

        # Expect database query
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None) #Expecting DBGetFields
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_FIELDS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), doid1) # ID
        self.assertEquals(dgi.read_uint16(), 2) # Field count
        self.assertEquals(dgi.read_uint16(), setDb3)
        self.assertEquals(dgi.read_uint16(), setRDB3)

        # Return field value to DBSS
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint32(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(99911)
        dg.add_uint16(setDb3)
        dg.add_string("He just really likes ice cream.")
        self.database.send(dg)

        # Expect field value from DBSS
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(3) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint32(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(99911)
        dg.add_uint16(setDb3)
        dg.add_string("He just really likes ice cream.")
        self.expect(self.shard, dg)



        ### Test for GetFields with mixed ram/non-ram db fields on inactive object ###
        # Query field from StateServer object
        dg = Datagram.create([doid1], 5, STATESERVER_OBJECT_GET_FIELDS)
        dg.add_uint32(2) # Context
        dg.add_doid(doid1) # ID
        dg.add_uint16(2) # Field count
        dg.add_uint16(setFoo)
        dg.add_uint16(setRDB3)
        self.shard.send(dg)

        # Expect database query
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None) #Expecting DBGetFields
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid1, DBSERVER_OBJECT_GET_FIELDS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), doid1) # ID
        self.assertEquals(dgi.read_uint16(), 1) # Field count
        self.assertEquals(dgi.read_uint16(), setRDB3)

        # Return field value to DBSS
        dg = Datagram.create([doid1], 1200, DBSERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(99922)
        self.database.send(dg)

        # Expect field value from DBSS
        dg = Datagram.create([5], doid1, STATESERVER_OBJECT_GET_FIELDS_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(1)
        dg.add_uint16(setRDB3)
        dg.add_uint32(99922)
        self.expect(self.shard, dg)

    def test_get_activated(self):
        self.shard.flush()
        self.database.flush()

        doid = 9050

        ### Test for GetActivated with non-existent or inactive object ###
        dg = Datagram.create([doid], 5, DBSS_OBJECT_GET_ACTIVATED)
        dg.add_uint32(1) # Context
        dg.add_doid(doid)
        self.shard.send(dg)

        # Expect false
        dg = Datagram.create([5], doid, DBSS_OBJECT_GET_ACTIVATED_RESP)
        dg.add_uint32(1) # Context
        dg.add_doid(doid)
        dg.add_uint8(BOOL_NO)
        self.expect(self.shard, dg)


        ### Test for GetActivated on loading object that doesn't exist ###
        # Activate object with defaults
        dg = Datagram.create([doid], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        dg.add_doid(doid) # Id
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid) # object Id

        # Ask if we are activated
        dg = Datagram.create([doid], 5, DBSS_OBJECT_GET_ACTIVATED)
        dg.add_uint32(2) # Context
        dg.add_doid(doid)
        self.shard.send(dg)

        # Send back to the DBSS with failure
        dg = Datagram.create([doid], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(FAILURE)
        self.database.send(dg)

        # Expect false
        dg = Datagram.create([5], doid, DBSS_OBJECT_GET_ACTIVATED_RESP)
        dg.add_uint32(2) # Context
        dg.add_doid(doid)
        dg.add_uint8(BOOL_NO)
        self.expect(self.shard, dg)


        ### Test for GetActivated on loading object that exists ###
        # Activate object with defaults
        dg = Datagram.create([doid], 5, DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS)
        dg.add_doid(doid) # Id
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        self.shard.send(dg)

        # Expect values to be retrieved from database
        dg = self.database.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1200], doid, DBSERVER_OBJECT_GET_ALL,
                                            remaining = 4 + DOID_SIZE_BYTES))
        context = dgi.read_uint32() # Get context
        self.assertEquals(dgi.read_doid(), doid) # object Id

        # Ask if we are activated
        dg = Datagram.create([doid], 5, DBSS_OBJECT_GET_ACTIVATED)
        dg.add_uint32(3) # Context
        dg.add_doid(doid)
        self.shard.send(dg)

        # Send back to the DBSS with some values
        dg = Datagram.create([doid], 1200, DBSERVER_OBJECT_GET_ALL_RESP)
        dg.add_uint32(context)
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject5)
        dg.add_uint16(2)
        dg.add_uint16(setRDB3)
        dg.add_uint32(777)
        dg.add_uint16(setRDbD5)
        dg.add_uint8(222)
        self.database.send(dg)

        # Expect true
        dg = Datagram.create([5], doid, DBSS_OBJECT_GET_ACTIVATED_RESP)
        dg.add_uint32(3) # Context
        dg.add_doid(doid)
        dg.add_uint8(BOOL_YES)
        self.expect(self.shard, dg)


        ### Test for GetActivated on activated object ### (continues from last)
        dg = Datagram.create([doid], 5, DBSS_OBJECT_GET_ACTIVATED)
        dg.add_uint32(4) # Context
        dg.add_doid(doid)
        self.shard.send(dg)

        # Expect true
        dg = Datagram.create([5], doid, DBSS_OBJECT_GET_ACTIVATED_RESP)
        dg.add_uint32(4) # Context
        dg.add_doid(doid)
        dg.add_uint8(BOOL_YES)
        self.expect(self.shard, dg)


        ### Test for GetActivated when killing the object ### (continues from last)
        dg = Datagram.create([doid], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(doid)
        self.shard.send(dg)

        # Is it still active?
        dg = Datagram.create([doid], 5, DBSS_OBJECT_GET_ACTIVATED)
        dg.add_uint32(5) # Context
        dg.add_doid(doid)
        self.shard.send(dg)

        # Expect false
        dg = Datagram.create([5], doid, DBSS_OBJECT_GET_ACTIVATED_RESP)
        dg.add_uint32(5) # Context
        dg.add_doid(doid)
        dg.add_uint8(BOOL_NO)
        self.expect(self.shard, dg)

if __name__ == '__main__':
    unittest.main()
