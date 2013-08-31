#!/usr/bin/env python2
import unittest
from socket import *

from common import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    connect: 127.0.0.1:57124
"""

CONTROL_CHANNEL = 4001
CONTROL_ADD_CHANNEL = 2001
CONTROL_REMOVE_CHANNEL = 2002

class TestMessageDirector(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        listener = socket(AF_INET, SOCK_STREAM)
        listener.bind(('127.0.0.1', 57124))
        listener.listen(1)
        listener.settimeout(0.3)

        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        l, _ = listener.accept()
        listener.close()
        cls.l1 = MDConnection(l)

        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        cls.c1 = MDConnection(c)
        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        cls.c2 = MDConnection(c)

    @classmethod
    def tearDownClass(cls):
        cls.l1.close()
        cls.c1.close()
        cls.c2.close()
        cls.daemon.stop()

    def test_single(self):
        self.l1.flush()

        # Send a datagram...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(1234)
        dg.add_uint64(4321)
        dg.add_uint16(1337)
        dg.add_string('HELLO')
        self.c1.send(dg)

        # Make sure the MD passes it upward.
        self.assertTrue(self.l1.expect(dg))

    def test_subscribe(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to a channel...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        dg.add_uint16(CONTROL_ADD_CHANNEL)
        dg.add_uint64(12345654321)
        self.c1.send(dg)
        self.assertTrue(self.c1.expect_none())
        # Make sure the MD subscribes to its parent.
        self.assertTrue(self.l1.expect(dg))

        # Send a test datagram on second connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(12345654321)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.assertTrue(self.c1.expect(dg))
        # MD will, of course, relay this datagram upward.
        self.assertTrue(self.l1.expect(dg))

        # Subscribe on the second connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        dg.add_uint16(CONTROL_ADD_CHANNEL)
        dg.add_uint64(12345654321)
        self.c2.send(dg)
        self.assertTrue(self.c2.expect_none())
        # MD should not ask for the channel a second time.
        self.assertTrue(self.l1.expect_none())

        # Send a test datagram on first connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(12345654321)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.c1.send(dg)
        self.assertTrue(self.c2.expect(dg)) # Should be relayed to second.
        self.assertTrue(self.l1.expect(dg)) # Should be sent upward.
        self.assertTrue(self.c1.expect_none()) # Should NOT be echoed back.

        # Unsubscribe on the first connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        dg.add_uint16(CONTROL_REMOVE_CHANNEL)
        dg.add_uint64(12345654321)
        self.c1.send(dg)
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect_none())
        # MD should NOT unsubscribe from parent!
        self.assertTrue(self.l1.expect_none())

        # Send another test datagram on second connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(12345654321)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.assertTrue(self.l1.expect(dg)) # Should be sent upward.
        self.assertTrue(self.c2.expect_none()) # Should NOT be relayed.
        self.assertTrue(self.c1.expect_none()) # Should NOT be echoed back.

        # Unsubscribe on the second connection...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        dg.add_uint16(CONTROL_REMOVE_CHANNEL)
        dg.add_uint64(12345654321)
        self.c2.send(dg)
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect_none())
        # MD should unsubscribe from parent.
        self.assertTrue(self.l1.expect(dg))

if __name__ == '__main__':
    unittest.main()
