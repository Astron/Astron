#!/usr/bin/env python2
import unittest, time
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

CONTROL_ADD_POST_REMOVE = 2010
CONTROL_CLEAR_POST_REMOVE = 2011

class TestMessageDirector(unittest.TestCase):
    @classmethod
    def new_connection(cls):
        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        return MDConnection(c)

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

        cls.c1 = cls.new_connection()
        cls.c2 = cls.new_connection()

    @classmethod
    def tearDownClass(cls):
        cls.l1.close()
        cls.c1.close()
        cls.c2.close()
        cls.daemon.stop()

    '''def test_single(self):
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
        self.assertTrue(self.l1.expect(dg))'''

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
        #self.assertTrue(self.c1.expect_none()) # Should NOT be echoed back.

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

    def test_multi(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to a pair of channels on c1.
        for channel in [1111, 2222]:
            dg = Datagram()
            dg.add_uint8(1)
            dg.add_uint64(CONTROL_CHANNEL)
            dg.add_uint16(CONTROL_ADD_CHANNEL)
            dg.add_uint64(channel)
            self.c1.send(dg)

        # Subscribe to another pair of channels on c2.
        for channel in [2222, 3333]:
            dg = Datagram()
            dg.add_uint8(1)
            dg.add_uint64(CONTROL_CHANNEL)
            dg.add_uint16(CONTROL_ADD_CHANNEL)
            dg.add_uint64(channel)
            self.c2.send(dg)

        self.l1.flush() # Don't care about the subscribe messages.

        # Sanity check: A datagram on channel 2222 should be delivered to both.
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(2222)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c2.expect(dg))

        # A datagram to channels 1111 and 3333 should be delievered to both.
        dg = Datagram()
        dg.add_uint8(2)
        dg.add_uint64(1111)
        dg.add_uint64(3333)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c2.expect(dg))

        # A datagram should only be delivered once if multiple channels match.
        dg = Datagram()
        dg.add_uint8(2)
        dg.add_uint64(1111)
        dg.add_uint64(2222)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect(dg))

        # Let's try something really absurd:
        dg = Datagram()
        dg.add_uint8(9)
        dg.add_uint64(1111)
        dg.add_uint64(2222)
        dg.add_uint64(3333)
        dg.add_uint64(1111)
        dg.add_uint64(1111)
        dg.add_uint64(2222)
        dg.add_uint64(3333)
        dg.add_uint64(3333)
        dg.add_uint64(2222)
        dg.add_uint64(0)
        dg.add_uint16(1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect(dg))
        self.assertTrue(self.c2.expect_none())

        # And, of course, sending this monstrosity on c1 should result in it
        # showing up on c2 and l1 once only; no echo back on c1.
        self.c1.send(dg)
        self.assertTrue(self.l1.expect(dg))
        self.assertTrue(self.l1.expect_none())
        self.assertTrue(self.c2.expect(dg))
        self.assertTrue(self.c2.expect_none())
        self.assertTrue(self.c1.expect_none())

        # Unsubscribe the channels...
        for channel in [1111, 2222, 3333]:
            dg = Datagram()
            dg.add_uint8(1)
            dg.add_uint64(CONTROL_CHANNEL)
            dg.add_uint16(CONTROL_REMOVE_CHANNEL)
            dg.add_uint64(channel)
            self.c1.send(dg)
            self.c2.send(dg)

    '''def test_post_remove(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Create a datagram to be sent post-remove...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(555444333)
        dg.add_uint64(0)
        dg.add_uint16(4321)
        dg.add_string('Testing...')

        # Hang it on c1...
        dg2 = Datagram()
        dg2.add_uint8(1)
        dg2.add_uint64(CONTROL_CHANNEL)
        dg2.add_uint16(CONTROL_ADD_POST_REMOVE)
        dg2.add_string(dg.get_data())
        self.c1.send(dg2)

        # Verify nothing's happening yet...
        self.assertTrue(self.l1.expect_none())
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect_none())

        # Reconnect c1 and see if dg gets sent.
        self.c1.close()
        self.__class__.c1 = self.new_connection()
        self.assertTrue(self.l1.expect(dg))

        # Hang dg as a post-remove for c2...
        self.c2.send(dg2)

        # Wait, nevermind...
        dg = Datagram()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        dg.add_uint16(CONTROL_CLEAR_POST_REMOVE)
        self.c2.send(dg)

        # Did the cancellation work?
        self.c2.close()
        self.__class__.c2 = self.new_connection()
        self.assertTrue(self.l1.expect_none())'''

if __name__ == '__main__':
    unittest.main()
