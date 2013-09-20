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

if __name__ == '__main__':
    unittest.main()
