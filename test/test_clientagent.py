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

if __name__ == '__main__':
    unittest.main()
