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

    def connect(self):
        s = socket(AF_INET, SOCK_STREAM)
        s.connect(('127.0.0.1', 57128))
        client = ClientConnection(s)

        return client

    def test_hello(self):
        # First, see if the CA ensures that the first datagram is a HELLO.
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(5) # invalid msgtype
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_NO_HELLO)

        # Next, see if the version gets validated:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(DC_HASH)
        dg.add_string('Equestria Online v5.7')
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_BAD_VERSION)

        # Now dchash validation:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(0x12345678)
        dg.add_string(VERSION)
        client.send(dg)
        self.assertDisconnect(client, CLIENT_DISCONNECT_BAD_DCHASH)

        # If everything is correct, it should simply allow us in:
        client = self.connect()
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO)
        dg.add_uint32(DC_HASH)
        dg.add_string(VERSION)
        client.send(dg)
        dg = Datagram()
        dg.add_uint16(CLIENT_HELLO_RESP)
        self.assertTrue(client.expect(dg))

        client.close()

if __name__ == '__main__':
    unittest.main()
