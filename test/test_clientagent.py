#!/usr/bin/env python2
import unittest, time
from socket import *

from common import *
from testdc import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

general:
    dc_files:
        - %r
uberdogs:
    - id: 1234
      class: UberDog1
      anonymous: true

    - id: 1235
      class: UberDog2
      anonymous: false

roles:
    - type: clientagent
      bind: 127.0.0.1:57128
      version: "Sword Art Online v5.1"
      channels:
          min: 100
          max: 999
      client:
          relocate: true

    - type: clientagent
      bind: 127.0.0.1:57135
      version: "Sword Art Online v5.1"
      channels:
          min: 110600
          max: 110699
""" % test_dc
VERSION = 'Sword Art Online v5.1'

class TestClientAgent(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        s = socket(AF_INET, SOCK_STREAM)
        s.connect(('127.0.0.1', 57123))
        cls.server = MDConnection(s)

        cls.server.send(Datagram.create_add_channel(1234))
        cls.server.send(Datagram.create_add_channel(1235))

    @classmethod
    def tearDownClass(cls):
        cls.server.close()
        cls.daemon.stop()

    def assertDisconnect(self, s, reason_code):
        while True:
            dg = s.recv_maybe()
            self.assertTrue(dg is not None, "No datagram received while expecting ClientEject")
            dgi = DatagramIterator(dg)
            if dgi.read_uint16() == CLIENT_EJECT:
                self.assertEqual(dgi.read_uint16(), reason_code)
                # According to client_protocol.md, the string must provide some explanation, so
                # ensure that it is there.
                self.assertNotEqual(dgi.read_string(), '')
                s.close()
                return

    def connect(self, do_hello=True, port=57128):
        s = socket(AF_INET, SOCK_STREAM)
        s.connect(('127.0.0.1', port))
        client = ClientConnection(s)

        if do_hello:
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO)
            dg.add_uint32(DC_HASH)
            dg.add_string(VERSION)
            client.send(dg)
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO_RESP)
            self.assertTrue(*client.expect(dg))

        return client

    def identify(self, client):
        # Figure out the sender ID for a given client connection.

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(request)
        dg.add_string('What... am I?')
        client.send(dg)

        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received when attempting identify()")
        dgi = DatagramIterator(dg)
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_channel(), 1234)
        sender_id = dgi.read_channel()

        #self.assertLessEqual(sender_id, 999)
        #self.assertGreaterEqual(sender_id, 100)

        return sender_id

    def set_state(self, client, state):
        dg = Datagram.create([self.identify(client)], 1, CLIENTAGENT_SET_STATE)
        dg.add_uint16(state)
        self.server.send(dg)
        time.sleep(0.1) # Mitigate race condition with set_state

    def test_hello(self):
        # First, see if the CA ensures that the first datagram is a HELLO.
        client = self.connect(False)
        dg = Datagram()
        dg.add_uint16(5) # invalid msgtype
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_NO_HELLO)

        # Next, see if the version gets validated:
        client = self.connect(False)
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(DC_HASH)
        dg.add_string('Equestria Online v5.7')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_BAD_VERSION)

        # Now dchash validation:
        client = self.connect(False)
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(0x12345678)
        dg.add_string(VERSION)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_BAD_DCHASH)

        # If everything is correct, it should simply allow us in:
        client = self.connect(False)
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(DC_HASH)
        dg.add_string(VERSION)
        client.send(dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO_RESP)
        self.assertTrue(*client.expect(dg))

        client.close()

    def test_anonymous(self):
        # Connect and hello:
        client = self.connect()

        # Now, we should be able to send an update to the anonymous UD...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(request)
        dg.add_string('[month of winter coolness]*3')
        client.send(dg)

        # Nothing should happen on the client...
        self.assertTrue(client.expect_none())

        # And the server should see the update...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received while expecting SSObjectSetField")
        dgi = DatagramIterator(dg)
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_channel(), 1234)
        dgi.read_channel() # Sender ID; we don't know this.
        self.assertEqual(dgi.read_uint16(), STATESERVER_OBJECT_SET_FIELD)
        self.assertEqual(dgi.read_doid(), 1234)
        self.assertEqual(dgi.read_uint16(), request)
        self.assertEqual(dgi.read_string(), '[month of winter coolness]*3')

        # However, if we send an update to the non-anonymous UD...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(foo)
        dg.add_uint8(1)
        dg.add_uint8(5)
        dg.add_uint8(7)
        client.send(dg)

        # Nothing should happen on the server...
        self.assertTrue(self.server.expect_none())

        # And the client should get booted.
        self.assertDisconnect(client, CLIENT_DISCONNECT_ANONYMOUS_VIOLATION)

    def test_disconnect(self):
        client = self.connect()
        id = self.identify(client)

        # Send a CLIENTAGENT_DISCONNECT to the session...
        dg = Datagram.create([id], 1, CLIENTAGENT_EJECT)
        dg.add_uint16(999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.server.send(dg)

        # See if the client dies with that exact error...
        dg = Datagram()
        dg.add_uint16(CLIENT_EJECT)
        dg.add_uint16(999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.assertTrue(*client.expect(dg))
        client.close()

        # New connection:
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, CLIENTAGENT_DROP)
        self.server.send(dg)

        # TODO: Rewrite this line to be less ugly.
        # Check that the connection closes with no other data.
        self.assertEqual(client.s.recv(1024), '')

    def test_set_state(self):
        client = self.connect()
        id = self.identify(client)

        # Restore the client back to the NEW state.
        self.set_state(client, CLIENT_STATE_NEW)

        # Since it's in the NEW state, sending something should cause a HELLO
        # error.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(request)
        dg.add_string('I wish to be an Oompa Loompa. Take me to them so the deed may be done.')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_NO_HELLO)

        # New client:
        client = self.connect()
        id = self.identify(client)

        # Put the client in the ESTABLISHED state.
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Try to send an update to UberDog2.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(foo)
        dg.add_uint8(0xBE)
        dg.add_uint8(0xAD)
        dg.add_uint8(0x06)
        client.send(dg)

        # The update should show up inside the server:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received while expecting SSObjectSetField")
        dgi = DatagramIterator(dg)
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_channel(), 1235)
        dgi.read_channel() # Sender ID; we don't know this.
        self.assertEqual(dgi.read_uint16(), STATESERVER_OBJECT_SET_FIELD)
        self.assertEqual(dgi.read_doid(), 1235)
        self.assertEqual(dgi.read_uint16(), foo)
        self.assertEqual(dgi.read_uint8(), 0xBE)
        self.assertEqual(dgi.read_uint8(), 0xAD)
        self.assertEqual(dgi.read_uint8(), 0x06)

        # Now revert back to anonymous state:
        self.set_state(client, CLIENT_STATE_ANONYMOUS)

        # Try again:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(foo)
        dg.add_uint8(0xBE)
        dg.add_uint8(0xAD)
        dg.add_uint8(0x06)
        client.send(dg)

        # Nothing should happen inside the server:
        self.assertTrue(self.server.expect_none())

        # Client should get booted:
        self.assertDisconnect(client, CLIENT_DISCONNECT_ANONYMOUS_VIOLATION)

    def test_receive_update(self):
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(response)
        dg.add_string('It... is... ON!')
        self.server.send(dg)

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(response)
        dg.add_string('It... is... ON!')
        self.assertTrue(*client.expect(dg))

        client.close()

    def test_set_sender(self):
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, CLIENTAGENT_SET_CLIENT_ID)
        dg.add_channel(555566667777)
        self.server.send(dg)

        # Reidentify the client, make sure the sender has changed.
        self.assertEquals(self.identify(client), 555566667777)

        # The client should have a subscription on the new sender channel automatically:
        dg = Datagram.create([555566667777], 1, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(0xF00D)
        self.server.send(dg)

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(0xF00D)
        self.assertTrue(*client.expect(dg))

        # Change the sender ID again... This is still being sent to the session
        # channel, which the client should always have a subscription on.
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_CLIENT_ID)
        dg.add_channel(777766665555)
        self.server.send(dg)

        # Ensure that 555566667777 has been dropped...
        dg = Datagram.create([555566667777], 1, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(0x7AB)
        self.server.send(dg)
        self.assertTrue(client.expect_none())

    def test_errors(self):
        # Zero-length datagram:
        client = self.connect()
        dg = Datagram()
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_TRUNCATED_DATAGRAM)

        # Really truncated datagram:
        client = self.connect()
        dg = Datagram()
        dg.add_uint8(3)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_TRUNCATED_DATAGRAM)

        # Bad msgtype:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(0x1337)
        dg.add_channel(0x3141592653589793) # Why the heck not add something to it?
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_INVALID_MSGTYPE)

        # Twiddle with an unknown object:
        client = self.connect()
        self.set_state(client, CLIENT_STATE_ESTABLISHED) # Let it out of the sandbox for this test.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_uint32(0xDECAFBAD)
        dg.add_uint16(request)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_MISSING_OBJECT)

        # Send a field update to a non-present field:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(foo)
        dg.add_uint8(5)
        dg.add_uint8(4)
        dg.add_uint8(6)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)

        # Send a field update to a non-clsend field:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(response)
        dg.add_string('Bizaam!')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)

        # Send a truncated field update:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1234)
        dg.add_uint16(request)
        dg.add_uint16(CHANNEL_SIZE_BYTES*2) # Faking the string length...
        dg.add_channel(0) # Whoops, only 8/16 bytes!
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_TRUNCATED_DATAGRAM)

        if not 'USE_32BIT_DATAGRAMS' in os.environ:
            # Send an oversized field update:
            client = self.connect()
            dg = Datagram()
            dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
            dg.add_uint32(1234)
            dg.add_uint16(request)
            # This will fit inside the client dg, but be too big for the server.
            dg.add_string('F'*(DGSIZE_MAX-len(dg._data)-DGSIZE_SIZE_BYTES))
            client.send(dg)
            self.assertDisconnect(client, CLIENT_DISCONNECT_OVERSIZED_DATAGRAM)

    def test_ownership(self):
        client = self.connect()
        id = self.identify(client)

        # Let the client out of the sandbox...
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Give it an object that it owns.
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_doid(55446655)
        dg.add_doid(1234) # Parent
        dg.add_zone(5678) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Big crown thingy')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.server.send(dg)

        # The client should receive the new object.
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER)
        dg.add_doid(55446655)
        dg.add_doid(1234) # Parent
        dg.add_zone(5678) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Big crown thingy')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.assertTrue(*client.expect(dg))

        # ownsend should be okay...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(setColor)
        dg.add_uint8(44)
        dg.add_uint8(55)
        dg.add_uint8(66)
        client.send(dg)
        self.assertTrue(client.expect_none())

        # clsend as well...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(requestKill)
        client.send(dg)
        self.assertTrue(client.expect_none())

        # And we can relocate it...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LOCATION)
        dg.add_doid(55446655) # Doid
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)
        self.assertTrue(client.expect_none())

        # But anything else is a no-no.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(setName)
        dg.add_string('Alicorn Amulet')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)


        # Send a relocate on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2)
        self.set_state(client2, CLIENT_STATE_ESTABLISHED)
        ## Give it an object that it owns.
        dg = Datagram.create([id2], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(88112288) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Bigger crown thingy from hell')
        dg.add_uint8(1)
        dg.add_uint8(2)
        dg.add_uint8(3)
        self.server.send(dg)
        ## The client should receive the new object.
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OWNER)
        dg.add_doid(88112288) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Bigger crown thingy from hell')
        dg.add_uint8(1)
        dg.add_uint8(2)
        dg.add_uint8(3)
        self.assertTrue(*client2.expect(dg))
        ## Lets try to relocate it...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LOCATION)
        dg.add_doid(88112288) # Doid
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client2.send(dg)
        ## Which should cause an error
        self.assertDisconnect(client2, CLIENT_DISCONNECT_FORBIDDEN_RELOCATE)

    def test_postremove(self):
        client = self.connect()
        id = self.identify(client)

        pr1 = Datagram.create([1234], 66, 12346)
        pr2 = Datagram.create([1234, 1235], 88, 12345)
        pr3 = Datagram.create([1235], 88, 6543)

        # Add a post-remove...
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_POST_REMOVE)
        dg.add_string(pr1.get_data())
        self.server.send(dg)

        # Clear the post-removes...
        dg = Datagram.create([id], 1, CLIENTAGENT_CLEAR_POST_REMOVES)
        self.server.send(dg)

        # Add two different ones...
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_POST_REMOVE)
        dg.add_string(pr2.get_data())
        self.server.send(dg)

        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_POST_REMOVE)
        dg.add_string(pr3.get_data())
        self.server.send(dg)

        #Wait
        time.sleep(0.1)

        # Client suddenly loses connection...!
        client.close()

        # Ensure pr2 and pr3 are sent out, but not pr1.
        self.assertTrue(*self.server.expect_multi([pr2, pr3], only=True))
        self.assertTrue(self.server.expect_none())

    def test_send_datagram(self):
        client = self.connect()
        id = self.identify(client)

        raw_dg = Datagram()
        raw_dg.add_uint16(5555)
        raw_dg.add_channel(152379565)

        dg = Datagram.create([id], 1, CLIENTAGENT_SEND_DATAGRAM)
        dg.add_string(raw_dg.get_data())
        self.server.send(dg)

        self.assertTrue(*client.expect(raw_dg))
        client.close()

    def test_channel(self):
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, CLIENTAGENT_OPEN_CHANNEL)
        dg.add_channel(66778899)
        self.server.send(dg)

        # In practice, it's dumb to do this, but this is a unit test, so:
        dg = Datagram.create([id], 1, CLIENTAGENT_CLOSE_CHANNEL)
        dg.add_channel(id)
        self.server.send(dg)

        # Sending things to the ID channel should no longer work...
        dg = Datagram.create([id], 1, CLIENTAGENT_EJECT)
        dg.add_uint16(1234)
        dg.add_string('What fun is there in making sense?')
        self.server.send(dg)

        # Nothing happens to the client:
        self.assertTrue(client.expect_none())

        # But the new channel is now valid!
        dg = Datagram.create([66778899], 1, CLIENTAGENT_EJECT)
        dg.add_uint16(4321)
        dg.add_string('What fun is there in making cents?')
        self.server.send(dg)

        self.assertDisconnect(client, 4321)

    def test_interest(self):
        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on two zones in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(2) # Context
        dg.add_uint16(1) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_uint16(2) # Zone count
        dg.add_zone(5555) # Zone 1
        dg.add_zone(4444) # Zone 2
        client.send(dg)

        # Server should ask for the objects:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 2)
        self.assertEquals(set([dgi.read_zone(), dgi.read_zone()]), set([5555, 4444]))

        # The SS replies immediately with the count:
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(2) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # We'll throw a couple objects its way:
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.assertTrue(*client.expect(dg))

        # Now the CA discovers the second object...
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string('What cause have I to feel glad?')
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER)
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string('What cause have I to feel glad?')
        self.assertTrue(*client.expect(dg))

        # And the CA is done opening the interest.

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint16(1) # Interest Id
        self.assertTrue(*client.expect(dg))

        # One of the objects broadcasts!
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4444], 1, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(7777) # do_id
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.server.send(dg)

        # And the client is informed.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(7777) # do_id
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.assertTrue(*client.expect(dg))

        # Now, let's move the objects around...
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|5555], 1, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # new_parent
        dg.add_zone(4444) # new_zone
        dg.add_doid(1234) # old_parent
        dg.add_zone(5555) # old_zone
        self.server.send(dg)

        # The client should get a zone change notification.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LOCATION)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        self.assertTrue(*client.expect(dg))

        # Realistically, object 8888 would send an enterzone on the new location:
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4444], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # But the CA should silently ignore it:
        self.assertTrue(client.expect_none())

        # How about moving the other object outside of interest?
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4444], 1, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # new_parent
        dg.add_zone(1111) # new_zone
        dg.add_doid(1234) # old_parent
        dg.add_zone(4444) # old_zone
        self.server.send(dg)

        # This time the client should have the object disabled.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(7777)
        self.assertTrue(*client.expect(dg))

        # Now, kill the interest!
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(55) # Context
        dg.add_uint16(1) # Interest id
        client.send(dg)

        # The remaining seen object should die...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(8888)
        self.assertTrue(*client.expect(dg))

        # The server should say it's done being interesting...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(55) # Context
        dg.add_uint16(1) # Interest id
        self.assertTrue(*client.expect(dg))

        # And NOTHING ELSE:
        self.assertTrue(client.expect_none())

        # Meanwhile, nothing happens on the server either:
        self.assertTrue(self.server.expect_none())

        # Additionally, if we try to twiddle with a previously visible object...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(8888)
        dg.add_uint16(setRequired1)
        dg.add_uint32(232323)
        client.send(dg)

        # We shouldn't get kicked...
        self.assertTrue(client.expect_none())

        self.server.flush()
        client.close()

    def test_delete(self):
        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_zone(111111) # Zone 1
        client.send(dg)

        # CA, of course, asks for objects:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235) # Parent
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 111111) # Zone #1

        # The SS replies immediately with the count:
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(1) #  Object count, uses integer with same width as doid
        self.server.send(dg)

        # We'll give them the object:
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER)
        dg.add_doid(777711) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(111111) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string("It's true some days are dark and lonely...")
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER)
        dg.add_doid(777711) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(111111) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string("It's true some days are dark and lonely...")
        self.assertTrue(*client.expect(dg))

        # And the client's interest op is done:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        self.assertTrue(*client.expect(dg))

        # Now let's nuke it from orbit!
        dg = Datagram.create([id], 777711, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(777711)
        self.server.send(dg)

        # Now the client should receive the deletion:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(777711)
        self.assertTrue(*client.expect(dg))

        # Next, we throw an owner object their way:
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_doid(55446651) # Object Id
        dg.add_doid(1235) # Parent
        dg.add_zone(6161) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Alicorn Amulet')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.server.send(dg)

        # The client should receive the new object.
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER)
        dg.add_doid(55446651) # Object Id
        dg.add_doid(1235) # Parent
        dg.add_zone(6161) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Alicorn Amulet')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.assertTrue(*client.expect(dg))

        # Bye, owned object!
        dg = Datagram.create([id], 55446651, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(55446651)
        self.server.send(dg)

        # Now the client should record an owner delete:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING_OWNER)
        dg.add_doid(55446651)
        self.assertTrue(*client.expect(dg))

        # That's all folks!
        client.close()

    def test_interest_overlap(self):
        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on two zones in 1235:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest Id
        dg.add_doid(1235) # Parent
        dg.add_uint16(2) # Zone count...
        dg.add_zone(1111) # Zone 1
        dg.add_zone(2222) # Zone 2
        client.send(dg)

        # CA asks for objects...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235) # Object doid
        self.assertEquals(dgi.read_uint16(), 2) # Nume zones requested
        dg.add_uint16(2)
        self.assertEquals(set([dgi.read_zone(), dgi.read_zone()]), set([1111, 2222]))

        # There is one object:
        dg = Datagram.create([id], 1235, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(1) # Object count, use integer with same width as doid
        self.server.send(dg)

        # Let's give 'em one...
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(54321) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(2222) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(54321) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(2222) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.assertTrue(*client.expect(dg))

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # context
        dg.add_uint16(5) # interest id
        self.assertTrue(*client.expect(dg))

        # Now, open a second, overlapping interest:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(8) # context 
        dg.add_uint16(7) # interest id
        dg.add_doid(1235) # parent
        dg.add_zone(2222) # Zone 2 from interest above.
        client.send(dg)

        # CA doesn't have to ask, this interest is already there.
        self.assertTrue(self.server.expect_none())

        # And it tells the client that the interest is open:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(8) # Context
        dg.add_uint16(7) # Interest id
        self.assertTrue(*client.expect(dg))

        # Now, the client asks to kill the first interest...
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(88) # Context
        dg.add_uint16(5) # Interest id
        client.send(dg)

        # And only the interest dies...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(88) # Context
        dg.add_uint16(5) # Interest id
        self.assertTrue(*client.expect(dg))

        # ...with no activity happening on the server.
        self.assertTrue(self.server.expect_none())

        # But if we kill the SECOND interest...
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(99) # Context
        dg.add_uint16(7) # Interest id
        client.send(dg)

        # ...the object dies...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(54321)
        self.assertTrue(*client.expect(dg))

        # ...the operation completes...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(99) # Context
        dg.add_uint16(7) # Interest id
        self.assertTrue(*client.expect(dg))

        # ...but still nothing on the server:
        self.assertTrue(self.server.expect_none())

        client.close()

    def test_alter_interest(self):
        # N.B. this is largely copied from the test above...

        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on two zones in 1235:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_uint16(2)
        dg.add_zone(1111) # Zone 1
        dg.add_zone(2222) # Zone 2
        client.send(dg)

        # CA asks for objects...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235)
        self.assertEquals(dgi.read_uint16(), 2)
        self.assertEquals(set([dgi.read_zone(), dgi.read_zone()]), set([1111, 2222]))

        # There is one object:
        dg = Datagram.create([id], 1235, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(1) # Object count, uses an integer with same width as doid
        self.server.send(dg)

        # Let's give 'em one...
        dg = Datagram.create([id], 54321, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(54321) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(2222) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(54321) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(2222) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.assertTrue(*client.expect(dg))

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        self.assertTrue(*client.expect(dg))

        # Now the client alters the interest to add a third zone:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(9) # Context
        dg.add_uint16(5) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_uint16(3) # Zone count
        dg.add_zone(1111) # Zone 1
        dg.add_zone(2222) # Zone 2
        dg.add_zone(5555) # Zone 5--er, 3...
        client.send(dg)

        # The CA should ask for JUST the difference...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235) # parent
        self.assertEquals(dgi.read_uint16(), 1) # zone count
        self.assertEquals(dgi.read_zone(), 5555) # zone #1

        # We'll pretend 1235,5555 is empty, so:
        dg = Datagram.create([id], 1235, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(0) # Object count, uses an integer with same width as doid
        self.server.send(dg)

        # And the CA should tell the client the handle/context operation is done:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(9) # Context
        dg.add_uint16(5) # Interest id
        self.assertTrue(*client.expect(dg))

        # Now let's alter to add another zone, but remove 2222:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(10) # Context
        dg.add_uint16(5) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_uint16(3) # Zone count
        dg.add_zone(5555) # zones requested out of their original order,
        dg.add_zone(1111) # because ordering is for suckers
        dg.add_zone(8888)
        client.send(dg)

        # The CA should ask for stuff in 8888...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235) # Parent
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 8888) # Zone #1

        # We'll pretend there's something in there this time:
        dg = Datagram.create([id], 1235, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(1) # Object count, uses an integer with same width as doid
        self.server.send(dg)

        dg = Datagram.create([id], 23239, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(23239) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(8888) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(31416) # setRequired1
        self.server.send(dg)

        # Now, the CA should say two things (not necessarily in order):
        expected = []
        # 1. Object 54321 is dead:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(54321)
        expected.append(dg)
        # 2. Object 23239 is alive:
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(23239) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(8888) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(31416) # setRequired1
        expected.append(dg)
        # Let's see if the CA does as it's supposed to:
        self.assertTrue(*client.expect_multi(expected))

        # And the CA should tell the client the handle/context operation is finished:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(10) # Context
        dg.add_uint16(5) # Interest id
        self.assertTrue(*client.expect(dg))

        # Now let's alter the interest to a different parent entirely:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(119) # Context
        dg.add_uint16(5) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(1111) # Zone
        client.send(dg)

        # The query goes out to the SS...
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234) # Parent
        self.assertEquals(dgi.read_uint16(), 1)  # Zone count
        self.assertEquals(dgi.read_zone(), 1111) # Zone

        # We'll pretend 1234,1111 is empty, so:
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(0) # Object count, uses an integer with same width as doid
        self.server.send(dg)

        # Now the CA destroys object 23239...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(23239)
        self.assertTrue(*client.expect(dg))

        # Interest operation finished:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(119) # Context
        dg.add_uint16(5)   # Interest id
        self.assertTrue(*client.expect(dg))

        # Cave Johnson, we're done here.
        client.close()

if __name__ == '__main__':
    unittest.main()
