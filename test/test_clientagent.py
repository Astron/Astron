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
            dg = s.recv()
            dgi = DatagramIterator(dg)
            if dgi.read_uint16() == CLIENT_GO_GET_LOST:
                self.assertEqual(dgi.read_uint16(), reason_code)
                s.close()
                return

    def connect(self, do_hello=True):
        s = socket(AF_INET, SOCK_STREAM)
        s.connect(('127.0.0.1', 57128))
        client = ClientConnection(s)

        if do_hello:
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO)
            dg.add_uint32(DC_HASH)
            dg.add_string(VERSION)
            client.send(dg)
            dg = Datagram()
            dg.add_uint16(CLIENT_HELLO_RESP)
            self.assertTrue(client.expect(dg))

        return client

    def identify(self, client):
        # Figure out the sender ID for a given client connection.

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(request)
        dg.add_string('What... am I?')
        client.send(dg)

        dgi = DatagramIterator(self.server.recv())
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_uint64(), 1234)
        sender_id = dgi.read_uint64()

        #self.assertLessEqual(sender_id, 999)
        #self.assertGreaterEqual(sender_id, 100)

        return sender_id

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
        self.assertTrue(client.expect(dg))

        client.close()

    def test_anonymous(self):
        # Connect and hello:
        client = self.connect()

        # Now, we should be able to send an update to the anonymous UD...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(request)
        dg.add_string('[month of winter coolness]*3')
        client.send(dg)

        # Nothing should happen on the client...
        self.assertTrue(client.expect_none())

        # And the server should see the update...
        dgi = DatagramIterator(self.server.recv())
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_uint64(), 1234)
        dgi.read_uint64() # Sender ID; we don't know this.
        self.assertEqual(dgi.read_uint16(), STATESERVER_OBJECT_UPDATE_FIELD)
        self.assertEqual(dgi.read_uint32(), 1234)
        self.assertEqual(dgi.read_uint16(), request)
        self.assertEqual(dgi.read_string(), '[month of winter coolness]*3')

        # However, if we send an update to the non-anonymous UD...
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
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
        dg = Datagram.create([id], 1, CLIENTAGENT_DISCONNECT)
        dg.add_uint16(999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.server.send(dg)

        # See if the client dies with that exact error...
        dg = Datagram()
        dg.add_uint16(CLIENT_GO_GET_LOST)
        dg.add_uint16(999)
        dg.add_string('ERROR: The night... will last... forever!')
        self.assertTrue(client.expect(dg))
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
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_STATE)
        dg.add_uint16(0)
        self.server.send(dg)

        # Since it's in the NEW state, sending something should cause a HELLO
        # error.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(request)
        dg.add_string('I wish to be an Oompa Loompa. Take me to them so the deed may be done.')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_NO_HELLO)

        # New client:
        client = self.connect()
        id = self.identify(client)

        # Put the client in the ESTABLISHED state.
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_STATE)
        dg.add_uint16(2)
        self.server.send(dg)

        # Try to send an update to UberDog2.
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
        dg.add_uint16(foo)
        dg.add_uint8(0xBE)
        dg.add_uint8(0xAD)
        dg.add_uint8(0x06)
        client.send(dg)

        # The update should show up inside the server:
        dgi = DatagramIterator(self.server.recv())
        self.assertEqual(dgi.read_uint8(), 1)
        self.assertEqual(dgi.read_uint64(), 1235)
        dgi.read_uint64() # Sender ID; we don't know this.
        self.assertEqual(dgi.read_uint16(), STATESERVER_OBJECT_UPDATE_FIELD)
        self.assertEqual(dgi.read_uint32(), 1235)
        self.assertEqual(dgi.read_uint16(), foo)
        self.assertEqual(dgi.read_uint8(), 0xBE)
        self.assertEqual(dgi.read_uint8(), 0xAD)
        self.assertEqual(dgi.read_uint8(), 0x06)

        # Now revert back to anonymous state:
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_STATE)
        dg.add_uint16(1)
        self.server.send(dg)

        # Try again:
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
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

        dg = Datagram.create([id], 1, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(response)
        dg.add_string('It... is... ON!')
        self.server.send(dg)

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(response)
        dg.add_string('It... is... ON!')
        self.assertTrue(client.expect(dg))

        client.close()

    def test_set_sender(self):
        client = self.connect()
        id = self.identify(client)

        dg = Datagram.create([id], 1, CLIENTAGENT_SET_SENDER_ID)
        dg.add_uint64(555566667777)
        self.server.send(dg)

        # Reidentify the client, make sure the sender has changed.
        self.assertTrue(self.identify(client), 555566667777)

        # The client should have a subscription on the new sender channel automatically:
        dg = Datagram.create([555566667777], 1, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
        dg.add_uint16(bar)
        dg.add_uint16(0xF00D)
        self.server.send(dg)

        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
        dg.add_uint16(bar)
        dg.add_uint16(0xF00D)
        self.assertTrue(client.expect(dg))

        # Change the sender ID again... This is still being sent to the session
        # channel, which the client should always have a subscription on.
        dg = Datagram.create([id], 1, CLIENTAGENT_SET_SENDER_ID)
        dg.add_uint64(777766665555)
        self.server.send(dg)

        # Ensure that 555566667777 has been dropped...
        dg = Datagram.create([555566667777], 1, STATESERVER_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1235)
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
        dg.add_uint64(0x3141592653589793) # Why the heck not add something to it?
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_INVALID_MSGTYPE)

        # Twiddle with an unknown object:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(0xDECAFBAD)
        dg.add_uint16(request)
        dg.add_string('Hello? Anyone there?')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_MISSING_OBJECT)

        # Send a field update to a non-present field:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(foo)
        dg.add_uint8(5)
        dg.add_uint8(4)
        dg.add_uint8(6)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)

        # Send a field update to a non-clsend field:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(response)
        dg.add_string('Bizaam!')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_FORBIDDEN_FIELD)

        # Send a truncated field update:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(request)
        dg.add_uint16(16) # Faking the string length...
        dg.add_uint64(0) # Whoops, only 8 bytes!
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_TRUNCATED_DATAGRAM)

        # Send an oversized field update:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_OBJECT_UPDATE_FIELD)
        dg.add_uint32(1234)
        dg.add_uint16(request)
        dg.add_string('F'*65525) # This will fit inside the client dg, but be too big for the server.
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_OVERSIZED_DATAGRAM)

if __name__ == '__main__':
    unittest.main()
