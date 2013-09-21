#!/usr/bin/env python2
from common import *
from testdc import *

CREATE_DOID_OFFSET = 1 + 8 + 8 + 2 + 4
VERIFY_DELETE_OBJECT = 0x21656944
VERIFY_DELETE_QUERY = 0x6c6c694b

class DatabaseBaseTests(object):
    def createTypeGetId(self, sender, context, type):
        # Create object of type
        dg = Datagram.create([777], sender, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(context)
        dg.add_uint16(type)
        dg.add_uint16(0) # Field count
        self.conn.send(dg)

        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        dgi.seek(CREATE_DOID_OFFSET)
        return dgi.read_uint32()

    def createGenericGetId(self, sender, context):
        return self.createTypeGetId(sender, context, DistributedTestObject1)

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
        dg.add_uint8(SUCCESS)
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
        self.assertTrue(dgi.read_uint8() == SUCCESS)
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
        dg.add_uint8(FAILURE_NOT_FOUND)
        self.assertTrue(self.conn.expect(dg))

        # Cleanup
        for doid in doids
            self.deleteObject(20, doid)
        self.conn.send(Datagram.create_remove_channel(20))

    def test_delete(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(30))

        # Create an object, get its doid
        doid = self.createGenericGetId(30, 1)

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
        dg.add_uint8(FAILURE_NOT_SUCCESS)
        self.assertTrue(self.conn.expect(dg)) # object deleted

        # Create some other objects
        doidA = self.createGenericGetId(30, 3)
        doidB = self.createGenericGetId(30, 4)

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
        dg.add_uint8(SUCCESS)
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
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint16(0) # Field count
        self.assertTrue(self.conn.expect(dg)) # object "B" not deleted

        # Cleanup
        self.deleteObject(30, doidB)
        self.conn.send(Datagram.create_remove_channel(30))

    def test_create_collisions(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(40))

        doids = []

        # Create the maximum number of objects we can assign
        doid = self.createGenericGetId(40, len(doids))
        while doid != 0 and len(doids) < 15:
            doids.append(doid)
            doid = self.createGenericGetId(40, len(doids))

        self.assertTrue(len(set(doids)) == len(doids)) # Check if duplicate do_ids exist
        self.assertTrue(len(doids) == 11) # Check we recieved the max do_ids we requested
        self.assertTrue(doid == INVALID_DO_ID) # Check the last object returned was BAD_DO_ID (0x0)

        # Delete an object
        self.deleteObject(40, doids[6])

        # Get new object with the last remaining id
        newdoid = self.createGenericGetId(40, 16)
        self.assertTrue(newdoid == doids[6])

        # Delete multiple objects
        self.deleteObject(40, doids[0])
        self.deleteObject(40, doids[1])
        self.deleteObject(40, doids[2])
        doids = doids[3:]

        # Create an object, it shouldn't collide
        doid = self.createGenericGetId(40, 17)
        for do in doids:
            self.assertTrue(do != doid)

        # Cleanup
        self.deleteObject(40, doid)
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

        def assert_no_change(context):
            # Retrieve object from the database...
            dg = Datagram.create([777], 50, DBSERVER_SELECT_STORED_OBJECT_ALL)
            dg.add_uint32(context) # Context
            dg.add_uint32(doid)
            self.conn.send(dg)

            # Get values back from server
            dg = self.conn.recv()
            dgi = DatagramIterator(dg)
            self.assertTrue(dgi.matches_header([50], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP))
            self.assertTrue(dgi.read_uint32() == context) # Check context
            self.assertTrue(dgi.read_uint8() == SUCCESS)
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

        # Create shouldn't store ram fields
        assert_no_change(2)

        # Update object with ram fields
        dg = Datagram.create([777], 50, DBSERVER_UPDATE_STORED_OBJECT)
        dg.add_uint32(doid)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setBR1)
        dg.add_string("(deep breath...) 'Yay...'")
        self.conn.send(dg)

        # Update shouldn't store ram fields
        assert_no_change(3)

        # Update if equals with a ram field
        dg = Datagram.create([777], 50, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(4) # Context
        dg.add_uint32(doid)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(91849) # Old value
        dg.add_uint32(44444) # New value
        dg.add_uint16(setBA1)
        dg.add_uint16(0) # Old value (null: 0)
        dg.add_uint16(239) # New value
        self.conn.send(dg)

        # Get update failure
        dg = Datagram.create([50], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(4) # Context
        dg.add_uint8(FAILURE_INVALID_FIELDS)
        self.conn.expect(dg)

        # Update shouldn't store ram fields, are update non-ram fields
        assert_no_change(5)

        # Cleanup
        self.deleteObject(50, doid)
        self.conn.send(Datagram.create_remove_channel(50))

    def test_update(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(60))

        # Create db object
        dg = Datagram.create([777], 60, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1)
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(54231)
        self.conn.send(dg)

        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        dgi.seek(CREATE_DOID_OFFSET)
        doid = dgi.read_uint32()

        # Select all fields from the stored object
        dg = Datagram.create([777], 60, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(2) # Context
        dg.add_uint32(doid)
        self.conn.send(dg)

        # Retrieve object from the database
        # Should get only RDB3 back
        dg = Datagram.create([60], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(54231)
        self.assertTrue(self.conn.expect(dg)) # Expecting SELECT_RESP with RDB3

        # Update values
        dg = Datagram.create([777], 60, DBSERVER_UPDATE_STORED_OBJECT)
        dg.add_uint32(doid)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setDb3)
        dg.add_string("20 percent cooler!!!")
        self.conn.send(dg)

        # Select all fields from the stored object
        dg = Datagram.create([777], 60, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(3) # Context
        dg.add_uint32(doid)
        self.conn.send(dg)

        # Retrieve object from the database
        # The values should be updated
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([60], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP))
        self.assertTrue(dgi.read_uint32() == 3) # Check context
        self.assertTrue(dgi.read_uint8() == SUCCESS)
        self.assertTrue(dgi.read_uint16() == DistributedTestObject3)
        self.assertTrue(dgi.read_uint16() == 2) # Check field count
        for x in xrange(2):
            field = dgi.read_uint16()
            if field == setRDB3:
                self.assertTrue(dgi.read_uint32() == 54231)
            elif field == setDb3:
                self.assertTrue(dgi.read_string() == "20 percent cooler!!!")
            else:
                self.fail("Bad field type")

        # Cleanup
        self.deleteObject(60, doid)
        self.conn.send(Datagram.create_remove_channel(60))

    def test_update_if_equals(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(70))

        # Create db object
        dg = Datagram.create([777], 70, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(767676)
        self.conn.send(dg)

        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        dgi.seek(CREATE_DOID_OFFSET)
        doid = dgi.read_uint32()

        # Update field with correct old value
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(2) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(767676) # Old value
        dg.add_uint32(787878) # New value
        self.conn.send(dg)

        # Get update response
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint8(SUCCESS)
        self.assertTrue(self.conn.expect(dg))

        # Select object with new value
        dg = Datagram.create([777], 70, DBSERVER_SELECT_STORED_OBJECT)
        dg.add_uint32(3) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        self.conn.send(dg)

        # Recieve updated value
        dg = Datagram.create([70], [777], DBSERVER_SELECT_STORED_OBJECT_RESP)
        dg.add_uint32(3) # Context
        dg.add_uint8(SUCCESS)
        dg.add_uint32(787878)
        self.assertTrue(self.conn.expect(dg))

        # Update field with incorrect old value
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(4) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(767676) # Old value (incorrect)
        dg.add_uint32(383838) # New value
        self.conn.send(dg)

        # Get update failure
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(4) # Context
        dg.add_uint8(FAILURE_VALUES_NOT_EQUAL)
        dg.add_uint32(787878) # Correct value
        self.assertTrue(self.conn.expect(dg))

        # Comparison existing value to non existing value in update
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(5) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setDb3)
        dg.add_string("That was a TERRIBLE surprise!") # Old value
        dg.add_string("Wish upon a twinkle...") # New value
        self.conn.send(dg)

        # Get update failure
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(5) # Context
        dg.add_uint8(FAILURE_VALUES_NOT_EQUAL)
        dg.add_uint16(0) # Non-existant value (0-length string)
        self.assertTrue(self.conn.expect(dg))

        # Update field where updated value equals the null value
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(6) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setDb3)
        dg.add_uint16(0) # Old value (null: 0-length)
        dg.add_string("Daddy... why did you eat my fries? I bought them... and they were mine.")
        self.conn.send(dg)

        # Get update success
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint8(SUCCESS)
        self.assertTrue(self.conn.expect(dg))

        # Select object with new value
        dg = Datagram.create([777], 70, DBSERVER_SELECT_STORED_OBJECT)
        dg.add_uint32(7) # Context
        dg.add_uint32(doid)
        dg.add_uint16(1) # Field count
        dg.add_uint16(setDb3)
        self.conn.send(dg)

        # Recieve updated value
        dg = Datagram.create([70], [777], DBSERVER_SELECT_STORED_OBJECT_RESP)
        dg.add_uint32(7) # Context
        dg.add_uint8(SUCCESS)
        dg.add_string("Daddy... why did you eat my fries? I bought them... and they were mine.")
        self.assertTrue(self.conn.expect(dg))

        # Update object with partially incorrect values
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(8) # Context
        dg.add_uint32(doid)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(787878) # Old value
        dg.add_uint32(919191) # New value
        dg.add_uint16(setDb3)
        dg.add_string("I can clear the sky in 10 seconds flat.")
        dg.add_string("Jesse!! We have to code!")

        # Get update failure
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(8) # Context
        dg.add_uint8(FAILURE_VALUES_NOT_EQUAL)
        dg.add_uint32(787878)
        dg.add_string("Daddy... why did you eat my fries? I bought them... and they were mine.")

        # Update multiple with correct old values
        dg = Datagram.create([777], 70, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS)
        dg.add_uint32(9) # Context
        dg.add_uint32(doid)
        dg.add_uint16(2) # Field count
        dg.add_uint16(setRDB3)
        dg.add_uint32(787878) # Old value
        dg.add_uint32(919191) # New value
        dg.add_uint16(setDb3)
        dg.add_string("Daddy... why did you eat my fries? I bought them... and they were mine.")
        dg.add_string("Mind if I... take a look inside the barn?!") # New value

        # Recieve update success
        dg = Datagram.create([70], 777, DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP)
        dg.add_uint32(9) # Context
        dg.add_uint8(SUCCESS)
        self.assertTrue(self.conn.expect(dg))

        # Select object with new value
        dg = Datagram.create([777], 70, DBSERVER_SELECT_STORED_OBJECT_ALL)
        dg.add_uint32(10) # Context
        dg.add_uint32(doid)
        self.conn.send(dg)

        # Recieve updated value
        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        self.assertTrue(dgi.matches_header([70], 777, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP))
        self.assertTrue(dgi.read_uint32() == 10) # Check context
        self.assertTrue(dgi.read_uint8() == SUCCESS)
        self.assertTrue(dgi.read_uint16() == DistributedTestObject3)
        self.assertTrue(dgi.read_uint16() == 2) # Check field count
        for x in xrange(2):
            field = dgi.read_uint16()
            if field == setRDB3:
                self.assertTrue(dgi.read_uint32() == 919191)
            elif field == setDb3:
                self.assertTrue(dgi.read_string() == "Mind if I... take a look inside the barn?!")
            else:
                self.fail("Bad field type")

        # Cleanup
        self.deleteObject(70, doid)
        self.conn.send(Datagram.create_remove_channel(70))

    def test_select(self):
        self.conn.flush()
        self.conn.send(Datagram.create_add_channel(80))

        # Create object
        dg = Datagram.create([777], 80, DBSERVER_CREATE_STORED_OBJECT)
        dg.add_uint32(1) # Context
        dg.add_uint16(DistributedTestObject3)
        dg.add_uint16(5) # Field count
        dg.add_uint16(setDb3)
        dg.add_string("Uppercut! Downercut! Fireball! Bowl of Punch!")
        self.conn.send(dg)

        dg = self.conn.recv()
        dgi = DatagramIterator(dg)
        dgi.seek(CREATE_DOID_OFFSET)
        doid = dgi.read_uint32()

        # Select the first field

        # Cleanup
        self.deleteObject(80, doid)
        self.conn.send(Datagram.create_remove_channel(80))