#!/usr/bin/env python2
import unittest, time, ssl, struct
from socket import socket, AF_INET, SOCK_STREAM, error as socket_error
from common.unittests import ProtocolTest
from common.astron import *
from common.dcfile import *
from common.tls import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    threaded: %s

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
          min: 3100
          max: 3999
      client:
          relocate: true
          add_interest: enabled
          write_buffer_size: 0
          write_timeout_ms: 0
      tuning:
          interest_timeout: 500

    - type: clientagent
      bind: 127.0.0.1:57135
      version: "Sword Art Online v5.1"
      channels:
          min: 110600
          max: 110699
      client:
          add_interest: disabled
          write_buffer_size: 0
          write_timeout_ms: 0

    - type: clientagent
      bind: 127.0.0.1:57144
      version: "Sword Art Online v5.1"
      client:
          add_interest: visible
          write_buffer_size: 0
          write_timeout_ms: 0
      channels:
          min: 220600
          max: 220699

    - type: clientagent
      bind: 127.0.0.1:57214
      version: "Sword Art Online v5.1"
      channels:
          min: 330600
          max: 330699
      tls:
          certificate: %r
          key_file: %r
          handshake_timeout: 200

    - type: clientagent
      bind: 127.0.0.1:57223
      version: "Sword Art Online v5.1"
      haproxy: true
      channels:
          min: 440600
          max: 440649

    - type: clientagent
      bind: 127.0.0.1:57224
      version: "Sword Art Online v5.1"
      haproxy: true
      channels:
          min: 440650
          max: 440699
      tls:
          certificate: %r
          key_file: %r

    - type: clientagent
      bind: 127.0.0.1:51201
      version: "Sword Art Online v5.1"
      channels:
          min: 3100
          max: 3999
      client:
          heartbeat_timeout: 1000

""" % (USE_THREADING, test_dc, server_crt, server_key, server_crt, server_key)
VERSION = 'Sword Art Online v5.1'

class TestClientAgent(ProtocolTest):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()
        cls.server = cls.connectToServer()
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

    def connect(self, do_hello=True, port=57128, tls_opts=None, proxy_header=None):
        s = socket(AF_INET, SOCK_STREAM)
        s.connect(('127.0.0.1', port))
        if proxy_header is not None:
            s.send(proxy_header)
        if tls_opts is not None:
            s = ssl.wrap_socket(s,**tls_opts)

        client = ClientConnection(s)

        if do_hello:
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO)
            dg.add_uint32(DC_HASH)
            dg.add_string(VERSION)
            client.send(dg)
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO_RESP)
            self.expect(client, dg, isClient = True)

        return client

    def identify(self, client, ignore_id = False, min=3100, max=3999):
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

        if not ignore_id:
            self.assertLessEqual(sender_id, max)
            self.assertGreaterEqual(sender_id, min)

        return sender_id

    def set_state(self, client, state):
        dg = Datagram.create([self.identify(client, ignore_id = True)], 1, CLIENTAGENT_SET_STATE)
        dg.add_uint16(state)
        self.server.send(dg)
        time.sleep(0.1) # Mitigate race condition with set_state

    def test_hello(self):
        self.server.flush()
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
        self.expect(client, dg, isClient = True)

        client.close()

    def test_anonymous(self):
        self.server.flush()
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
        self.expectNone(client)

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
        self.expectNone(self.server)

        # And the client should get booted.
        self.assertDisconnect(client, CLIENT_DISCONNECT_ANONYMOUS_VIOLATION)

    def test_disconnect(self):
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Send a CLIENTAGENT_DISCONNECT to the session...
        dg = Datagram.create([id], 1, CLIENTAGENT_EJECT)
        dg.add_uint16(3999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.server.send(dg)

        # See if the client dies with that exact error...
        dg = Datagram()
        dg.add_uint16(CLIENT_EJECT)
        dg.add_uint16(3999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.expect(client, dg, isClient = True)
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
        self.server.flush()
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
        self.expectNone(self.server)

        # Client should get booted:
        self.assertDisconnect(client, CLIENT_DISCONNECT_ANONYMOUS_VIOLATION)

    def test_receive_update(self):
        self.server.flush()
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
        self.expect(client, dg, isClient = True)

        dg = Datagram.create([id], 1, STATESERVER_OBJECT_SET_FIELDS)
        dg.add_doid(1235)
        dg.add_uint16(2) # Field count
        dg.add_uint16(foo) # Field: foo
        dg.add_uint8(109)
        dg.add_uint8(111)
        dg.add_uint8(113)
        dg.add_uint16(bar) # Field: bar
        dg.add_uint16(8118)
        self.server.send(dg)

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELDS)
        dg.add_doid(1235)
        dg.add_uint16(2) # Field count
        dg.add_uint16(foo) # Field: foo
        dg.add_uint8(109)
        dg.add_uint8(111)
        dg.add_uint8(113)
        dg.add_uint16(bar) # Field: bar
        dg.add_uint16(8118)
        self.expect(client, dg, isClient = True)

        client.close()

    def test_set_sender(self):
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, CLIENTAGENT_SET_CLIENT_ID)
        dg.add_channel(555566667777)
        self.server.send(dg)
        self.server.flush()

        # Reidentify the client, make sure the sender has changed.
        self.assertEquals(self.identify(client, ignore_id = True), 555566667777)

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
        self.expect(client, dg, isClient = True)

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
        self.expectNone(client)

    def test_errors(self):
        self.server.flush()
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
        dg.add_doid(0xDECAFBAD) # Doid
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
            dg.add_doid(1234) # Doid
            dg.add_uint16(request)
            # This will fit inside the client dg, but be too big for the server.
            dg.add_string('F'*(DGSIZE_MAX-len(dg._data)-DGSIZE_SIZE_BYTES))
            client.send(dg)
            self.assertDisconnect(client, CLIENT_DISCONNECT_OVERSIZED_DATAGRAM)

    def test_ownership(self):
        self.server.flush()
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
        self.expect(client, dg, isClient = True)

        # ownsend should be okay...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(setColor)
        dg.add_uint8(44)
        dg.add_uint8(55)
        dg.add_uint8(66)
        client.send(dg)
        self.expectNone(client)

        # clsend as well...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(requestKill)
        client.send(dg)
        self.expectNone(client)

        # And we can relocate it...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LOCATION)
        dg.add_doid(55446655) # Doid
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)
        self.expectNone(client)

        # But anything else is a no-no.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(55446655)
        dg.add_uint16(setName)
        dg.add_string('Alicorn Amulet')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)



        ### Test loss of ownership when receiving CHANGE_OWNER message
        # Ok, we want another client to play with
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Give it an object that it owns.
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_doid(11226655)
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
        dg.add_doid(11226655)
        dg.add_doid(1234) # Parent
        dg.add_zone(5678) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Big crown thingy')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.expect(client, dg, isClient = True)

        # Then change the object's owner
        dg = Datagram.create([id], 11226655, STATESERVER_OBJECT_CHANGING_OWNER)
        dg.add_doid(11226655)
        dg.add_channel(0)
        dg.add_channel(id)
        self.server.send(dg)

        # Now the client should record an owner delete:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING_OWNER)
        dg.add_doid(11226655)
        self.expect(client, dg, isClient = True)



        ### Send a relocate on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2, min = 110600, max = 110699)
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
        self.expect(client2, dg, isClient = True)
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
        self.server.flush()
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
        self.expectMany(self.server, [pr2, pr3])
        self.expectNone(self.server)

    def test_send_datagram(self):
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        raw_dg = Datagram()
        raw_dg.add_uint16(5555)
        raw_dg.add_channel(152379565)

        dg = Datagram.create([id], 1, CLIENTAGENT_SEND_DATAGRAM)
        dg.add_string(raw_dg.get_data())
        self.server.send(dg)

        self.expect(client, raw_dg, isClient = True)
        client.close()

    def test_channel(self):
        self.server.flush()
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
        self.expectNone(client)

        # But the new channel is now valid!
        dg = Datagram.create([66778899], 1, CLIENTAGENT_EJECT)
        dg.add_uint16(4321)
        dg.add_string('What fun is there in making cents?')
        self.server.send(dg)

        self.assertDisconnect(client, 4321)

    def test_interest_undercounting(self):
        # The point of this test is to make sure the client agent handles objects
        # entering the zone in which interest is opened upon before the
        # interest is actually completed
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # The same N.B. in test_interest applies here

        # The CA should've asked for objects in some zones
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # We now tell the CA that there are 2 objects in the zone
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(2) # Object count
        self.server.send(dg)

        # The client shouldn't have heard anything back yet
        self.expectNone(client)

        # Now begins actual object entry into the interest's zone. Start with a contextually linked object.
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # The CA should buffer the entry of 8888...
        self.expectNone(client)

        # Now, some other object shows up into the same zone that is NOT linked to the interest
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8887) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # The client shouldn't hear about anything yet
        self.expectNone(client)

        # Alright, now send the last object entry
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # The CA should now have heard about everything it needed to conclude the iop
        # The client should hear about the contextual generates first, then a DONE_INTEREST,
        # and then the unrelated object's entry
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

        # All related interest objects have been sent, so we should be told that we're done there
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest Id
        self.expect(client, dg, isClient = True)

        # Last, other DG's should come in, so we should be hearing about the unrelated object
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8887) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

    def test_interest_timeout(self):
        # Test the interest timeout
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # The same N.B. in test_interest applies here

        # The CA should've asked for objects in some zones
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # We now tell the CA that there are 2 objects in the zone
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(2) # Object count
        self.server.send(dg)

        # The client shouldn't have heard anything back yet
        self.expectNone(client)

        # Send one object in
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # The CA should buffer the entry of 8888...
        self.expectNone(client)

        # Now, nothing happens for 750 ms
        time.sleep(0.75)

        # Interest timeout (500ms) should've hit, so the interest should've been closed.

        # We should hear about 8888...
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

        # ...and then be told that we're done with the interest.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest Id
        self.expect(client, dg, isClient = True)

    def test_interest_ignore_early_generate(self):
        # The point of this test is to make sure the ClientAgent safely handles
        # receiving an ENTER_INTEREST before a ZONES_COUNT_RESP.
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # The same N.B. in test_interest applies here

        # The CA should've asked for objects in some zones
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # Now, one of our objects enters prematurely
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        dg.add_uint16(1) # field count
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.server.send(dg)

        # We now tell the CA that there is 1 object in the zone
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count
        self.server.send(dg)

        # The Client Agent already got its one object, so it should finish up
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        dg.add_uint16(1) # field count
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.expect(client, dg, isClient = True)

        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest Id
        self.expect(client, dg, isClient = True)

        # Then we shouldn't expect any more datagrams
        self.expectNone(client)

    def test_interest_ignore_early_location(self):
        # The point of this test is to make sure the ClientAgent ignores normal
        # incoming messages for a location until it receives ZONES_COUNT_RESP.
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # The same N.B. in test_interest applies here

        # The CA should've asked for objects in some zones
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # Now, one of our objects enters prematurely
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # Not sure what the correct behavior here is.  May need to track the
        # difference between objects in the snapshot and other enters
        ## We also receive a set field for it!
        #dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4321], 1, STATESERVER_OBJECT_SET_FIELD)
        #dg.add_doid(8888) # do_id
        #dg.add_uint16(setBR1)
        #dg.add_string("I've built my life on judgement and causing pain...")
        #self.server.send(dg)

        # We now tell the CA that there is 1 object in the zone
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count
        self.server.send(dg)

        # The client shouldn't have heard anything back yet
        self.expectNone(client)

        # Now the object enters as an interest operation
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        dg.add_uint16(1) # field count
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.server.send(dg)

        # The Client Agent got its one object, so it should finish up
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        dg.add_uint16(1) # field count
        dg.add_uint16(setBR1)
        dg.add_string("I've built my life on judgement and causing pain...")
        self.expect(client, dg, isClient = True)

        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest Id
        self.expect(client, dg, isClient = True)

        # Then we shouldn't expect any more datagrams
        self.expectNone(client)

    def test_interest_client_relocate(self):
        # This test makes sure we handle the case where change location messages originate
        # from the client during an interest operation.  The Client Agent message should handle
        # these gracefully, and ignore the messages forwarded from the StateServer.
        self.server.flush()
        self.server.send(Datagram.create_add_channel(88111651))

        client = self.connect()
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # The client owns an object it can relocate
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER)
        dg.add_doid(88111651) # Object Id
        dg.add_doid(1235) # Parent
        dg.add_zone(6161) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Alicorn Amulet')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.server.send(dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER)
        dg.add_doid(88111651) # Object Id
        dg.add_doid(1235) # Parent
        dg.add_zone(6161) # Zone
        dg.add_uint16(DistributedClientTestObject)
        dg.add_string('Alicorn Amulet')
        dg.add_uint8(11)
        dg.add_uint8(22)
        dg.add_uint8(33)
        dg.add_uint16(0)
        self.expect(client, dg, isClient = True)

        # The Client tells the object to change location
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LOCATION)
        dg.add_doid(88111651) # Doid
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # Server expecting to receive the changing location message
        dg = Datagram.create([88111651], id, STATESERVER_OBJECT_SET_LOCATION);
        dg.add_doid(1234) # parent
        dg.add_doid(4321) # zone
        self.expect(self.server, dg)

        # Open interest on a currently empty zone, we'll be moving something in shortly
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2855) # Context
        dg.add_uint16(1855) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # The CA should've asked for objects in some zones
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # The StateServer would dispatch CHANGING_LOCATIONs first
        locations = [(1234<<ZONE_SIZE_BITS)|4321, (1235<<ZONE_SIZE_BITS)|6161]
        dg = Datagram.create(locations, id, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(88111651) # do_id
        dg.add_doid(1234) # new_parent
        dg.add_zone(4321) # new_zone
        dg.add_doid(1235) # old_parent
        dg.add_zone(6161) # old_zone
        self.server.send(dg)

        # Because the changing_location was generated by us, we should ignore it
        self.expectNone(client)

        # We now tell the CA that there is 1 objects in the zone
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count
        self.server.send(dg)

        # The client shouldn't have heard anything back yet
        self.expectNone(client)

        # Now send the ENTER_INTEREST message
        dg = Datagram.create([id], 88111651, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # This should finalize the operation
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2855) # Context
        dg.add_uint16(1855) # Interest Id
        self.expect(client, dg, isClient = True)

        # And receive nothing else
        self.expectNone(client)

    def test_interest(self):
        self.server.flush()
        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on a zone in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # Server should ask for the objects:
        # N. B. This is not the current behavior of the ClientAgent, but its also not the point
        #       of the test I'm trying to add because the stateserver doesn't actually implement
        #       STATESERVER_OBJECT_GET_ZONE_OBJECTS, so I'll re-enable this part later when that
        #       has been implemented, and we want to enforce that the behavior has changed
        #dg = self.server.recv_maybe()
        #self.assertTrue(dg is not None)
        #dgi = DatagramIterator(dg)
        #self.assertTrue(*dgi.matches_header([2345], id, STATESERVER_OBJECT_GET_ZONE_OBJECTS))
        #ss_context = dgi.read_uint32()
        #self.assertEquals(dgi.read_doid(), 1234)
        #self.assertEquals(dgi.read_zone(), 4321)

        ## The SS replies immediately with the count:
        #dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONE_COUNT_RESP)
        #dg.add_uint32(ss_context)
        #dg.add_doid(0) # Object count, uses an integer with same width as doids
        #self.server.send(dg)

        # The server asks for objects this way instead for now
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # In our first test, we'll open an interest containing no objects.
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(0) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2000) # Context
        dg.add_uint16(1000) # Interest Id
        self.expect(client, dg, isClient = True)

        # Now, kill the interest!
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(2001) # Context
        dg.add_uint16(1000) # Interest id
        client.send(dg)

        # The server should say it's done being interesting...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2001) # Context
        dg.add_uint16(1000) # Interest id
        self.expect(client, dg, isClient = True)

        # Now lets try this again, but this time send actual objects
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
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(context) # request_context
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # The client shouldn't hear about it yet
        self.expectNone(client)

        # Now the CA discovers the second object...
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER)
        dg.add_uint32(context) # request_context
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string('What cause have I to feel glad?')
        self.server.send(dg)

        # The CA has heard about all the objects it wanted, so now it should send the generates down
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # And the CA is done opening the interest.

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(2) # Context
        dg.add_uint16(1) # Interest Id
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # Realistically, object 8888 would send an enterzone on the new location:
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4444], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # But the CA should silently ignore it:
        self.expectNone(client)

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
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # The server should say it's done being interesting...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(55) # Context
        dg.add_uint16(1) # Interest id
        self.expect(client, dg, isClient = True)

        # And NOTHING ELSE:
        self.expectNone(client)

        # Meanwhile, nothing happens on the server either:
        self.expectNone(self.server)

        # Additionally, if we try to twiddle with a previously visible object...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(8888)
        dg.add_uint16(setRequired1)
        dg.add_uint32(232323)
        client.send(dg)

        # We shouldn't get kicked...
        self.expectNone(client)

        # Send an add interest on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2, min = 110600, max = 110699)
        self.set_state(client2, CLIENT_STATE_ESTABLISHED)
        # Open interest on two zones in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(707) # Context
        dg.add_uint16(505) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_zone(400000) # Zone
        client2.send(dg)

        # Which should cause an error
        self.assertDisconnect(client2, CLIENT_DISCONNECT_FORBIDDEN_INTEREST)

        # Send a remove interest on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2, min = 110600, max = 110699)
        self.set_state(client2, CLIENT_STATE_ESTABLISHED)
        # Add interest from server
        dg = Datagram.create([id2], 1, CLIENT_ADD_INTEREST)
        dg.add_uint16(2) # Interest id
        # ... and ignore all the actual interest messages

        # Send an add interest multiple on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2, min = 110600, max = 110699)
        self.set_state(client2, CLIENT_STATE_ESTABLISHED)
        # Open interest on two zones in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(4) # Context
        dg.add_uint16(1) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_uint16(2) # Zone count
        dg.add_zone(5555) # Zone 1
        dg.add_zone(4444) # Zone 2
        client2.send(dg)

        # Which should cause an error
        self.assertDisconnect(client2, CLIENT_DISCONNECT_FORBIDDEN_INTEREST)

        # Send a remove interest on a client agent that has it disabled
        client2 = self.connect(port = 57135)
        id2 = self.identify(client2, min = 110600, max = 110699)
        self.set_state(client2, CLIENT_STATE_ESTABLISHED)
        # Add interest from server
        dg = Datagram.create([id2], 1, CLIENT_ADD_INTEREST)
        dg.add_uint16(2) # Interest id
        # ... and ignore all the actual interest messages
        client2.flush()
        self.server.flush()

        # Remove some interest
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(0xF00) # Context
        dg.add_uint16(2) # Interest id
        client2.send(dg)

        # Which should cause an error
        self.assertDisconnect(client2, CLIENT_DISCONNECT_FORBIDDEN_INTEREST)

        # Send an add interest on a client agent that allows only visible on a
        # visble object (in this case an uberdog object)
        client3 = self.connect(port = 57144)
        id3 = self.identify(client3, min = 220600, max = 220699)
        self.set_state(client3, CLIENT_STATE_ESTABLISHED)
        # Open interest on two zones in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(81) # Context
        dg.add_uint16(101) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_uint16(2) # Zone count
        dg.add_zone(5555) # Zone 1
        dg.add_zone(4444) # Zone 2
        client3.send(dg)

        # Server should ask for the objects:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id3, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 2)
        self.assertEquals(set([dgi.read_zone(), dgi.read_zone()]), set([5555, 4444]))

        # The SS replies immediately with the count:
        dg = Datagram.create([id3], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(context)
        dg.add_doid(0) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(81) # Context
        dg.add_uint16(101) # Interest Id
        self.expect(client3, dg, isClient = True)

        # Send a remove interest
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(82) # Context
        dg.add_uint16(101) # Interest id
        client3.send(dg)

        # The server should say it's done being interesting...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(82) # Context
        dg.add_uint16(101) # Interest id
        self.expect(client3, dg, isClient = True)

        # We shouldn't get kicked, or receive anything else
        self.expectNone(client3)

        # Send an add interest on a client agent that allows only visible on a
        # an object that is not visible to the client.
        client3 = self.connect(port = 57144)
        id3 = self.identify(client3, min = 220600, max = 220699)
        self.set_state(client3, CLIENT_STATE_ESTABLISHED)
        # Open interest on two zones in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint32(83) # Context
        dg.add_uint16(102) # Interest id
        dg.add_doid(10000) # Parent
        dg.add_uint16(4) # Zone count
        dg.add_zone(1010101) # Zone 1
        dg.add_zone(2010101) # Zone 2
        dg.add_zone(3010101) # Zone 3
        dg.add_zone(4010101) # Zone 4
        client3.send(dg)

        # Which should cause an error
        self.assertDisconnect(client3, CLIENT_DISCONNECT_FORBIDDEN_INTEREST)

        # Cleanup
        self.server.flush()
        client.close()
        client2.close()
        client3.close()

    def test_delete(self):
        self.server.flush()
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
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER)
        dg.add_uint32(context)
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
        self.expect(client, dg, isClient = True)

        # And the client's interest op is done:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        self.expect(client, dg, isClient = True)

        # Now let's nuke it from orbit!
        dg = Datagram.create([id], 777711, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(777711)
        self.server.send(dg)

        # Now the client should receive the deletion:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(777711)
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # Bye, owned object!
        dg = Datagram.create([id], 55446651, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(55446651)
        self.server.send(dg)

        # Now the client should record an owner delete:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING_OWNER)
        dg.add_doid(55446651)
        self.expect(client, dg, isClient = True)

        # That's all folks!
        client.close()

    def test_interest_overlap(self):
        self.server.flush()
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
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(context)
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
        self.expect(client, dg, isClient = True)

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # context
        dg.add_uint16(5) # interest id
        self.expect(client, dg, isClient = True)

        # Now, open a second, overlapping interest:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(8) # context
        dg.add_uint16(7) # interest id
        dg.add_doid(1235) # parent
        dg.add_zone(2222) # Zone 2 from interest above.
        client.send(dg)

        # CA doesn't have to ask, this interest is already there.
        self.expectNone(self.server)

        # And it tells the client that the interest is open:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(8) # Context
        dg.add_uint16(7) # Interest id
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # ...with no activity happening on the server.
        self.expectNone(self.server)

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
        self.expect(client, dg, isClient = True)

        # ...the operation completes...
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(99) # Context
        dg.add_uint16(7) # Interest id
        self.expect(client, dg, isClient = True)

        # ...but still nothing on the server:
        self.expectNone(self.server)

        client.close()

    def test_alter_interest(self):
        # N.B. this is largely copied from the test above...
        self.server.flush()

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
        dg = Datagram.create([id], 54321, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(context)
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
        self.expect(client, dg, isClient = True)

        # So the CA should tell the client the handle/context operation is done.
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(6) # Context
        dg.add_uint16(5) # Interest id
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

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

        dg = Datagram.create([id], 23239, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(context)
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
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # Interest operation finished:
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(119) # Context
        dg.add_uint16(5)   # Interest id
        self.expect(client, dg, isClient = True)

        # Cave Johnson, we're done here.
        client.close()

    def test_serverside_interest(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(1015))

        # N. B. This is mostly copied from test_interest replacing
        #       CLIENT_ADD_INTEREST with CLIENTAGENT_ADD_INTEREST
        client = self.connect()
        id = self.identify(client)

        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Open interest on two zones in 1234:
        dg = Datagram.create([id], 1015, CLIENTAGENT_ADD_INTEREST_MULTIPLE)
        dg.add_uint16(12) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_uint16(2) # Zone count
        dg.add_zone(5555) # Zone 1
        dg.add_zone(4444) # Zone 2
        self.server.send(dg)

        # Client should have the interest forwarded back to it
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received while expecting fowarded interest.")
        dgi = DatagramIterator(dg)
        self.assertEquals(dgi.read_uint16(), CLIENT_ADD_INTEREST_MULTIPLE) # MsgType
        cl_context = dgi.read_uint32()
        self.assertEquals(dgi.read_uint16(), 12) # Interest id
        self.assertEquals(dgi.read_doid(), 1234) # Parent
        self.assertEquals(dgi.read_uint16(), 2) # Zone count
        # We don't actually care what order the zones are in
        zones = [5555, 4444]
        received_zone = dgi.read_zone()
        self.assertTrue(received_zone in zones) # Zone 1
        zones.remove(received_zone)
        received_zone = dgi.read_zone()
        self.assertTrue(received_zone in zones) # Zone 2

        # Server should ask for the objects:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 2)
        self.assertEquals(set([dgi.read_zone(), dgi.read_zone()]), set([5555, 4444]))

        # The SS replies immediately with the count:
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(2) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # We'll throw a couple objects its way:
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # Now the CA discovers the second object...
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED_OTHER)
        dg.add_uint32(ss_context)
        dg.add_doid(7777) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(88888) # setRequired1
        dg.add_uint16(1)
        dg.add_uint16(setBR1)
        dg.add_string('What cause have I to feel glad?')
        self.server.send(dg)

        # And the CA is done opening the interest. The CA should be clearing the buffer now.

        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(5555) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # So the CA should tell the client and caller the handle/context operation is done.
        dg = Datagram.create([1015], id, CLIENTAGENT_DONE_INTEREST_RESP)
        dg.add_channel(id) # Client id
        dg.add_uint16(12)  # Interest id
        self.expect(self.server, dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(cl_context) # Context
        dg.add_uint16(12) # Interest Id
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

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
        self.expect(client, dg, isClient = True)

        # Realistically, object 8888 would send an enterzone on the new location:
        dg = Datagram.create([(1234<<ZONE_SIZE_BITS)|4444], 1, STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
        dg.add_doid(8888) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4444) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(999999) # setRequired1
        self.server.send(dg)

        # But the CA should silently ignore it:
        self.expectNone(client)

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
        self.expect(client, dg, isClient = True)

        # Now, kill the interest!
        dg = Datagram.create([id], 1015, CLIENTAGENT_REMOVE_INTEREST)
        dg.add_uint16(12) # Interest id
        self.server.send(dg)

        # And the client should be informed
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received while expecting remove interest.")
        dgi = DatagramIterator(dg)
        self.assertEquals(dgi.read_uint16(), CLIENT_REMOVE_INTEREST) # MsgType
        cl_context = dgi.read_uint32()
        self.assertEquals(dgi.read_uint16(), 12) # Interest id

        # The remaining seen object should die...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING)
        dg.add_doid(8888)
        self.expect(client, dg, isClient = True)

        # The server should say it's done being interesting...
        dg = Datagram.create([1015], id, CLIENTAGENT_DONE_INTEREST_RESP)
        dg.add_channel(id) # Client id
        dg.add_uint16(12)  # Interest id
        self.expect(self.server, dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(cl_context) # Context
        dg.add_uint16(12) # Interest id
        self.expect(client, dg, isClient = True)

        # And NOTHING ELSE:
        self.expectNone(client)

        # Meanwhile, nothing happens on the server either:
        self.expectNone(self.server)

        # Additionally, if we try to twiddle with a previously visible object...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(8888)
        dg.add_uint16(setRequired1)
        dg.add_uint32(232323)
        client.send(dg)

        # We shouldn't get kicked...
        self.expectNone(client)
        # but the client agent should also ignore it
        self.expectNone(self.server)

        # Ok, we should also try to do some of this for add_interest
        dg = Datagram.create([id], 1015, CLIENTAGENT_ADD_INTEREST)
        dg.add_uint16(17) # Interest id
        dg.add_doid(1235) # Parent
        dg.add_zone(8888) # Zone
        self.server.send(dg)

        # Client should have the interest forwarded back to it
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "No datagram received while expecting fowarded interest.")
        dgi = DatagramIterator(dg)
        self.assertEquals(dgi.read_uint16(), CLIENT_ADD_INTEREST) # MsgType
        cl_context = dgi.read_uint32()
        self.assertEquals(dgi.read_uint16(), 17) # Interest id
        self.assertEquals(dgi.read_doid(), 1235) # Parent
        self.assertEquals(dgi.read_zone(), 8888) # Zone

        # Server should ask for the objects:
        # N. B. This is not the current behavior of the ClientAgent, but its also not the point
        #       of the test I'm trying to add because the stateserver doesn't actually implement
        #       STATESERVER_OBJECT_GET_ZONE_OBJECTS, so I'll re-enable this part later when that
        #       has been implemented, and we want to enforce that the behavior has changed
        #dg = self.server.recv_maybe()
        #self.assertTrue(dg is not None)
        #dgi = DatagramIterator(dg)
        #self.assertTrue(*dgi.matches_header([2345], id, STATESERVER_OBJECT_GET_ZONE_OBJECTS))
        #ss_context = dgi.read_uint32()
        #self.assertEquals(dgi.read_doid(), 2345)
        #self.assertEquals(dgi.read_zone(), 8888)

        ## The SS replies immediately with the count:
        #dg = Datagram.create([id], 2345, STATESERVER_OBJECT_GET_ZONE_COUNT_RESP)
        #dg.add_uint32(ss_context)
        #dg.add_doid(1) # Object count, uses an integer with same width as doids
        #self.server.send(dg)

        # INSTEAD WE DO -- Server should ask for the objects:
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1235], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1235)
        self.assertEquals(dgi.read_uint16(), 1)
        self.assertEquals(dgi.read_zone(), 8888)
        # The SS replies immediately with the count:
        dg = Datagram.create([id], 1235, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # We'll throw an object its way:
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context)
        dg.add_doid(343536) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(8888) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(219129) # setRequired1
        self.server.send(dg)

        # Does the client see it?
        dg = Datagram()
        dg.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED)
        dg.add_doid(343536) # do_id
        dg.add_doid(1235) # parent_id
        dg.add_zone(8888) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(219129) # setRequired1
        self.expect(client, dg, isClient = True)

        # The server should say it's done being interesting...
        dg = Datagram.create([1015], id, CLIENTAGENT_DONE_INTEREST_RESP)
        dg.add_channel(id) # Client id
        dg.add_uint16(17) # Interest id
        self.expect(self.server, dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_DONE_INTEREST_RESP)
        dg.add_uint32(cl_context) # Context
        dg.add_uint16(17) # Interest id
        self.expect(client, dg, isClient = True)

        # And NOTHING ELSE:
        self.expectNone(client)

        # Meanwhile, nothing happens on the server either:
        self.expectNone(self.server)
        self.server.send(Datagram.create_remove_channel(1015))

        self.server.flush()
        client.close()

    def test_declare(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(10000))

        # Declare a client
        client = self.connect()
        id = self.identify(client)

        # Ok declare on object for the client
        dg = Datagram.create([id], 1, CLIENTAGENT_DECLARE_OBJECT)
        dg.add_doid(10000) # doid
        dg.add_uint16(DistributedClientTestObject) # dclass
        self.server.send(dg)

        # Mitigate race condition with declare_object
        time.sleep(0.1)

        # Twiddle with the object, and get disconnected because we're not authenticate
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(sendMessage)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_ANONYMOUS_VIOLATION)
        self.expectNone(self.server)

        # Lets connect again
        client = self.connect()
        id = self.identify(client)
        # Client needs to be outside of the sandbox for this:
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Give us the same object
        dg = Datagram.create([id], 1, CLIENTAGENT_DECLARE_OBJECT)
        dg.add_doid(10000) # doid
        dg.add_uint16(DistributedClientTestObject) # dclass
        self.server.send(dg)

        # Mitigate race condition with declare_object
        time.sleep(0.1)

        # Twiddle with the object, and everything should run fine
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(sendMessage)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)

        # Expect the message forwarded to the object
        dg = Datagram.create([10000], id, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(sendMessage)
        dg.add_string('Hello? Anyone there?')
        self.expect(self.server, dg)

        # Then undeclare the object
        dg = Datagram.create([id], 1, CLIENTAGENT_UNDECLARE_OBJECT)
        dg.add_doid(10000) # doid
        self.server.send(dg)

        # Mitigate race condition with undeclare_object
        time.sleep(0.1)

        # Twiddle with the object, and get disconnected because its not declared
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(sendMessage)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_MISSING_OBJECT)
        self.expectNone(self.server)

        # Cleanup
        self.server.send(Datagram.create_remove_channel(10000))

    def test_fields_sendable(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(10000))

        # Declare a client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)
        client.flush()

        # Ok declare on object for the client
        dg = Datagram.create([id], 1, CLIENTAGENT_DECLARE_OBJECT)
        dg.add_doid(10000) # doid
        dg.add_uint16(DistributedTestObject1) # dclass
        self.server.send(dg)

        # Then lets set a field sendable
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_FIELDS_SENDABLE)
        dg.add_doid(10000) # doid
        dg.add_uint16(2) # num fields
        dg.add_uint16(setRequired1)
        dg.add_uint16(setBR1)
        self.server.send(dg)

        # Mitigate race condition with set_fields_sendable and declare_object
        time.sleep(0.1)

        # Try a SetField
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(setBR1)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)
        self.expectNone(client)

        # Expect the message forwarded to the object
        dg = Datagram.create([10000], id, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(setBR1)
        dg.add_string('Hello? Anyone there?')
        self.expect(self.server, dg)

        # Remove one of the set fields
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_FIELDS_SENDABLE)
        dg.add_doid(10000) # doid
        dg.add_uint16(1) # num fields
        dg.add_uint16(setRequired1)
        self.server.send(dg)

        # Mitigate race condition with set_fields_sendable
        time.sleep(0.1)

        # Make sure setRequired1 is still sendable
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xBA55)
        client.send(dg)
        self.expectNone(client)

        # Expect the message forwarded to the object
        dg = Datagram.create([10000], id, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(setRequired1)
        dg.add_uint32(0xBA55)
        self.expect(self.server, dg)

        # But trying to send to the other field should disconnect us....
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(10000)
        dg.add_uint16(setBR1)
        dg.add_string('When you are tired, tea is the best remedy. Also at all other times.')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)
        self.expectNone(self.server)

        # Ok now lets try to do it with an uberdog
        self.server.send(Datagram.create_remove_channel(10000))
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)
        client.flush()

        # Then lets set a field sendable
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_FIELDS_SENDABLE)
        dg.add_doid(1235) # doid
        dg.add_uint16(1) # num fields
        dg.add_uint16(bar)
        self.server.send(dg)

        # Mitigate race condition with set_fields_sendable
        time.sleep(0.1)

        # Try a SetField
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(323)
        client.send(dg)
        self.expectNone(client)

        # Expect the message forwarded to the object
        dg = Datagram.create([1235], id, STATESERVER_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(323)
        self.expect(self.server, dg)

        # Then clear the fields
        dg = Datagram.create([id], 0, CLIENTAGENT_SET_FIELDS_SENDABLE)
        dg.add_doid(1235) # doid
        dg.add_uint16(0) # num fields
        self.server.send(dg)

        # Mitigate race condition with set_fields_sendable
        time.sleep(0.1)

        # Trying to set the field should disconnect us then
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_SET_FIELD)
        dg.add_doid(1235)
        dg.add_uint16(bar)
        dg.add_uint16(323)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)
        self.expectNone(self.server)

    def test_session_objects(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(10052))
        self.server.send(Datagram.create_add_channel(10053))

        ### Lets test the behavior of a client exiting with one session object
        # Give us a client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Give it ownership of the object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Then disconnect the client, and the object should die
        client.close()
        dg = Datagram.create([10052], id, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.expect(self.server, dg)



        ### Lets test a session object being deleted with one object
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Give it ownership of the object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Then delete the object, the client should be disconnected
        dg = Datagram.create([id], 10052, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.server.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_SESSION_OBJECT_DELETED)
        # The server sent the delete, it shouldn't receive a message
        self.expectNone(self.server)



        ### Lets test a session object leaving ownership with one object
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Give it ownership of the object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Then change the object's owner, the client should be disconnected
        dg = Datagram.create([id], 10052, STATESERVER_OBJECT_CHANGING_OWNER)
        dg.add_doid(10052)
        dg.add_channel(0)
        dg.add_channel(id)
        self.server.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_SESSION_OBJECT_DELETED)
        # Additionally the object should be deleted
        dg = Datagram.create([10052], id, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.expect(self.server, dg)



        ### Lets test a session object leaving interest with one object
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Open interest on a zone in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(5000) # Context
        dg.add_uint16(3000) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # Server should ask for the objects:
        # N. B. This is not the current behavior of the ClientAgent, but its also not the point
        #       of the test I'm trying to add because the stateserver doesn't actually implement
        #       STATESERVER_OBJECT_GET_ZONE_OBJECTS, so I'll re-enable this part later when that
        #       has been implemented, and we want to enforce that the behavior has changed
        #dg = self.server.recv_maybe()
        #self.assertTrue(dg is not None)
        #dgi = DatagramIterator(dg)
        #self.assertTrue(*dgi.matches_header([2345], id, STATESERVER_OBJECT_GET_ZONE_OBJECTS))
        #ss_context = dgi.read_uint32()
        #self.assertEquals(dgi.read_doid(), 1234)
        #self.assertEquals(dgi.read_zone(), 4321)

        ## The SS replies immediately with the count:
        #dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONE_COUNT_RESP)
        #dg.add_uint32(ss_context)
        #dg.add_doid(0) # Object count, uses an integer with same width as doids
        #self.server.send(dg)

        # The server asks for objects this way instead for now
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # The interest contains our object
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # Pass the object from the stateserver
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context)
        dg.add_doid(10052) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(709907) # setRequired1
        self.server.send(dg)

        # Ignore the enter location and done interest response.
        client.flush()

        # Then remove the client's interest in the object, the client should be disconnected
        dg = Datagram()
        dg.add_uint16(CLIENT_REMOVE_INTEREST)
        dg.add_uint32(5001) # Context
        dg.add_uint16(3000) # Interest id
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_SESSION_OBJECT_DELETED)
        # Additionally the object should be deleted
        dg = Datagram.create([10052], id, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.expect(self.server, dg)



        ### Lets test a session object changing location out of interest
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Open interest on a zone in 1234:
        dg = Datagram()
        dg.add_uint16(CLIENT_ADD_INTEREST)
        dg.add_uint32(5002) # Context
        dg.add_uint16(3001) # Interest id
        dg.add_doid(1234) # Parent
        dg.add_zone(4321) # Zone
        client.send(dg)

        # Server should ask for the objects:
        # N. B. This is not the current behavior of the ClientAgent, but its also not the point
        #       of the test I'm trying to add because the stateserver doesn't actually implement
        #       STATESERVER_OBJECT_GET_ZONE_OBJECTS, so I'll re-enable this part later when that
        #       has been implemented, and we want to enforce that the behavior has changed
        #dg = self.server.recv_maybe()
        #self.assertTrue(dg is not None)
        #dgi = DatagramIterator(dg)
        #self.assertTrue(*dgi.matches_header([2345], id, STATESERVER_OBJECT_GET_ZONE_OBJECTS))
        #ss_context = dgi.read_uint32()
        #self.assertEquals(dgi.read_doid(), 1234)
        #self.assertEquals(dgi.read_zone(), 4321)

        ## The SS replies immediately with the count:
        #dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONE_COUNT_RESP)
        #dg.add_uint32(ss_context)
        #dg.add_doid(0) # Object count, uses an integer with same width as doids
        #self.server.send(dg)

        # The server asks for objects this way instead for now
        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None)
        dgi = DatagramIterator(dg)
        self.assertTrue(*dgi.matches_header([1234], id, STATESERVER_OBJECT_GET_ZONES_OBJECTS))
        ss_context = dgi.read_uint32()
        self.assertEquals(dgi.read_doid(), 1234)
        self.assertEquals(dgi.read_uint16(), 1) # Zone count
        self.assertEquals(dgi.read_zone(), 4321)

        # The interest contains our object
        dg = Datagram.create([id], 1234, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP)
        dg.add_uint32(ss_context)
        dg.add_doid(1) # Object count, uses an integer with same width as doids
        self.server.send(dg)

        # Pass the object from the stateserver
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_INTEREST_WITH_REQUIRED)
        dg.add_uint32(ss_context)
        dg.add_doid(10052) # do_id
        dg.add_doid(1234) # parent_id
        dg.add_zone(4321) # zone_id
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(709907) # setRequired1
        self.server.send(dg)

        # Ignore the enter location and done interest response.
        client.flush()

        # Wait a little bit to mitigate any potential race condition
        time.sleep(0.1)

        # Then change the object's location, out of interest, the client should be disconnected
        dg = Datagram.create([id], 10052, STATESERVER_OBJECT_CHANGING_LOCATION)
        dg.add_doid(10052)
        dg.add_doid(0) # new parent
        dg.add_zone(0) # new zone
        dg.add_doid(1234) # old parent
        dg.add_zone(4321) # old zone
        self.server.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_SESSION_OBJECT_DELETED)
        # Additionally the object should be deleted
        dg = Datagram.create([10052], id, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.expect(self.server, dg)



        ### Lets test a session object being deleted with multiple session objects
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Declare another session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10053) # doid
        self.server.send(dg)

        # Give it ownership of the first object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Give it ownership of the second object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10053) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xB0053) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Then delete the object, the client should be disconnected
        dg = Datagram.create([id], 10052, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.server.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_SESSION_OBJECT_DELETED)
        # The server should expect a delete for the second object
        dg = Datagram.create([10053], id, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10053)
        self.expect(self.server, dg)



        ### Lets make sure we don't get notified about object deletions after removing session objs
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Give it ownership of the object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")

        # Remove that object as a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_REMOVE_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Wait a little bit to mitigate any potential race condition
        time.sleep(0.1)

        # Then disconnect the client, and the object shouldn't die
        client.close()
        self.expectNone(self.server)



        ### Lets make sure we don't get dc'd about by a delete object after removing session objs
        # Give us another client
        client = self.connect()
        id = self.identify(client)
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        # Declare a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_ADD_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Give it ownership of the object
        dg = Datagram.create([id], 1, STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED)
        dg.add_doid(10052) # Doid
        dg.add_doid(0) # Parent
        dg.add_zone(0) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(0xBEE) # setRequired1
        self.server.send(dg)

        # Ignore the data in the entry forwarded to the client, we're not testing that here
        dg = client.recv_maybe()
        self.assertTrue(dg is not None, "The client didn't receive a datagram after ENTER_OWNER.")
        client.flush()

        # Remove that object as a session object
        dg = Datagram.create([id], 1, CLIENTAGENT_REMOVE_SESSION_OBJECT)
        dg.add_doid(10052) # doid
        self.server.send(dg)

        # Wait a little bit to mitigate any potential race condition
        time.sleep(0.1)

        # Then delete the object, the client should not be disconnected
        dg = Datagram.create([id], 10052, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_doid(10052)
        self.server.send(dg)
        # We should receive an object leaving owner
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_LEAVING_OWNER)
        dg.add_doid(10052)
        self.expect(client, dg, isClient = True)
        # But we shouldn't get disconnected
        self.expectNone(client)
        # The server sent the delete, so it shouldn't get anything back
        self.expectNone(self.server)


        ### Cleanup after all the tests
        self.server.send(Datagram.create_remove_channel(10052))
        self.server.send(Datagram.create_remove_channel(10053))

    def test_ssl_tls(self):
        self.server.flush()

        # Declare a client
        tls_context = {'ssl_version': ssl.PROTOCOL_TLSv1}
        client = self.connect(port = 57214, tls_opts = tls_context)
        id = self.identify(client, min = 330600, max = 330699)

    def test_ssl_tls_timeout(self):
        client = self.connect(port = 57214, do_hello=False)
        self.expectNone(client)
        time.sleep(0.2)

        # Client should now be dropped. It will either socket.error or return
        # '' when we try to recv from it (depending on platform):
        try:
            r = client.s.recv(1024)
        except socket_error as e:
            self.assertEqual(e.errno, 10054)
        else:
            self.assertEqual(r, '')

    def test_get_network_address(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(10052))

        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 10052, CLIENTAGENT_GET_NETWORK_ADDRESS)
        dg.add_uint32(1337)
        self.server.send(dg)

        dg = self.server.recv_maybe()
        self.assertTrue(dg is not None, "The server didn't receive a datagram. Expecting CLIENTAGENT_GET_NETWORK_ADDRESS_RESP")

        dgi = DatagramIterator(dg)
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_channel(), 10052)
        self.assertEqual(dgi.read_channel(), id)
        self.assertEqual(dgi.read_uint16(), CLIENTAGENT_GET_NETWORK_ADDRESS_RESP)
        self.assertEqual(dgi.read_uint32(), 1337)
        self.assertEqual(dgi.read_string(), "127.0.0.1")
        dgi.read_uint16() # Ignore remote port (can't really test this)
        self.assertEqual(dgi.read_string(), "127.0.0.1")
        dgi.read_uint16() # Ignore local port (can't really test this)

        self.server.send(Datagram.create_remove_channel(10052))

    def test_haproxy_protocol(self):
        self.server.flush()
        self.server.send(Datagram.create_add_channel(10010))

        for test_id in xrange(8):
            proto_v2 = bool(test_id&1)
            ipv6 = bool(test_id&2)
            tls = bool(test_id&4)

            if ipv6:
                source_ip = '2001:db8::1'
                source_ip_bin = '20010db8000000000000000000000001'.decode('hex')
                dest_ip = '2001:db8:dead:beef::feed:1'
                dest_ip_bin = '20010db8deadbeef00000000feed0001'.decode('hex')
            else:
                source_ip = '203.0.113.5'
                source_ip_bin = struct.pack('BBBB', 203, 0, 113, 5)
                dest_ip = '198.51.100.77'
                dest_ip_bin = struct.pack('BBBB', 198, 51, 100, 77)

            source_port = 54321
            dest_port = 12345

            if proto_v2:
                body = (source_ip_bin + dest_ip_bin +
                        struct.pack('>HH', source_port, dest_port))
                header = ('\r\n\r\n\0\r\nQUIT\n\x21' +
                          struct.pack('>BH', 0x21 if ipv6 else 0x11, len(body)) +
                          body)
            else:
                header = 'PROXY TCP{} {} {} {} {}\r\n'.format(
                    6 if ipv6 else 4, source_ip, dest_ip, source_port, dest_port)

            if tls:
                tls_context = {'ssl_version': ssl.PROTOCOL_TLSv1}
                client = self.connect(port=57224, proxy_header=header, tls_opts=tls_context)
            else:
                client = self.connect(port=57223, proxy_header=header)

            id = self.identify(client, min=440600, max=440699)
            dg = Datagram.create([id], 10010, CLIENTAGENT_GET_NETWORK_ADDRESS)
            dg.add_uint32(test_id)
            self.server.send(dg)

            dg = self.server.recv_maybe()
            self.assertTrue(dg is not None, "The server didn't receive a datagram. Expecting CLIENTAGENT_GET_NETWORK_ADDRESS_RESP")

            dgi = DatagramIterator(dg)
            self.assertEqual(dgi.read_uint8(), 1)
            self.assertEqual(dgi.read_channel(), 10010)
            self.assertEqual(dgi.read_channel(), id)
            self.assertEqual(dgi.read_uint16(), CLIENTAGENT_GET_NETWORK_ADDRESS_RESP)
            self.assertEqual(dgi.read_uint32(), test_id)
            self.assertEqual(dgi.read_string(), source_ip)
            self.assertEqual(dgi.read_uint16(), source_port)
            self.assertEqual(dgi.read_string(), dest_ip)
            self.assertEqual(dgi.read_uint16(), dest_port)

        self.server.send(Datagram.create_remove_channel(10010))

    # Test the interest timeout
    # The heartbeat timeout is configured for 1000ms (1 second)
    def test_heartbeat_timeout(self):
        #setup
        self.server.flush()
        client = self.connect(port = 51201)
        id = self.identify(client)

        # Bring client out of the sandbox
        self.set_state(client, CLIENT_STATE_ESTABLISHED)

        #setup our testing DG that will go test(server) -> MD -> CA -> Test(Client)
        #A successful receival of this DG on the Test(client) side will denote an 
        #active socket.
        raw_dg = Datagram("Hello World")
        dg = Datagram.create([id], 1, CLIENTAGENT_SEND_DATAGRAM)
        dg.add_string(raw_dg.get_data())

        #inital connection test
        self.server.send(dg)
        self.expect(client, raw_dg, isClient = True)


        #sleep for 3/4 of a second, still should be connected after this.
        time.sleep(0.75)

        #Check for connection and send heartbeat
        self.server.send(dg)
        self.expect(client, raw_dg, isClient = True)
        self.send_heartbeat(client)

        time.sleep(1.1)

        # We should be disconnected now...
        self.assertDisconnect(client,CLIENT_DISCONNECT_NO_HEARTBEAT)

        

    def send_heartbeat(self, client):
        # Construct heartbeat datagram
        heartbeat_dg = Datagram()
        heartbeat_dg.add_uint16(CLIENT_HEARTBEAT)
        client.send(heartbeat_dg) #send


if __name__ == '__main__':
    unittest.main()
