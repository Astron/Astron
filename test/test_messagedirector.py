#!/usr/bin/env python2
import unittest
from socket import *

from common import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    connect: 127.0.0.1:57124
"""

class TestMessageDirector(unittest.TestCase):
    @classmethod
    def new_connection(cls):
        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        return MDConnection(c)

    @classmethod
    def setUpClass(cls):
        listener = socket(AF_INET, SOCK_STREAM)
        listener.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
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

    def test_single(self):
        self.l1.flush()

        # Send a datagram...
        dg = Datagram.create([1234], 4321, 1337)
        dg.add_string('HELLO')
        self.c1.send(dg)

        # Make sure the MD passes it upward.
        self.assertTrue(self.l1.expect(dg))

    def test_subscribe(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to a channel...
        dg = Datagram.create_add_channel(12345654321)
        self.c1.send(dg)
        self.assertTrue(self.c1.expect_none())
        # Make sure the MD subscribes to its parent.
        self.assertTrue(self.l1.expect(dg))

        # Send a test datagram on second connection...
        dg = Datagram()
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.assertTrue(self.c1.expect(dg))
        # MD will, of course, relay this datagram upward.
        self.assertTrue(self.l1.expect(dg))

        # Subscribe on the second connection...
        dg = Datagram.create_add_channel(12345654321)
        self.c2.send(dg)
        self.assertTrue(self.c2.expect_none())
        # MD should not ask for the channel a second time.
        self.assertTrue(self.l1.expect_none())

        # Send a test datagram on first connection...
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c1.send(dg)
        self.assertTrue(self.c2.expect(dg)) # Should be relayed to second.
        self.assertTrue(self.l1.expect(dg)) # Should be sent upward.
        #self.assertTrue(self.c1.expect_none()) # Should NOT be echoed back.

        # Unsubscribe on the first connection...
        dg = Datagram.create_remove_channel(12345654321)
        self.c1.send(dg)
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect_none())
        # MD should NOT unsubscribe from parent!
        self.assertTrue(self.l1.expect_none())

        # Send another test datagram on second connection...
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.assertTrue(self.l1.expect(dg)) # Should be sent upward.
        self.assertTrue(self.c2.expect_none()) # Should NOT be relayed.
        self.assertTrue(self.c1.expect_none()) # Should NOT be echoed back.

        # Abandon the second connection, which should auto-unsubscribe it.
        self.c2.close()
        self.__class__.c2 = self.new_connection()
        self.assertTrue(self.c1.expect_none())
        # MD should unsubscribe from parent.
        self.assertTrue(self.l1.expect(Datagram.create_remove_channel(12345654321)))

    def test_multi(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to a pair of channels on c1.
        for channel in [1111, 2222]:
            dg = Datagram.create_add_channel(channel)
            self.c1.send(dg)

        # Subscribe to another pair of channels on c2.
        for channel in [2222, 3333]:
            dg = Datagram.create_add_channel(channel)
            self.c2.send(dg)

        self.l1.flush() # Don't care about the subscribe messages.

        # Sanity check: A datagram on channel 2222 should be delivered to both.
        dg = Datagram.create([2222], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c2.expect(dg))

        # A datagram to channels 1111 and 3333 should be delievered to both.
        dg = Datagram.create([1111, 3333], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c2.expect(dg))

        # A datagram should only be delivered once if multiple channels match.
        dg = Datagram.create([1111, 2222], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c1.expect_none())
        self.assertTrue(self.c2.expect(dg))

        # Let's try something really absurd:
        dg = Datagram.create([1111, 2222, 3333, 1111, 1111,
                              2222, 3333, 3333, 2222], 0, 1234)
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
            dg = Datagram.create_remove_channel(channel)
            self.c1.send(dg)
            self.c2.send(dg)

    def test_post_remove(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Create a datagram to be sent post-remove...
        dg = Datagram()
        dg = Datagram.create([555444333], 0, 4321)
        dg.add_string('Testing...')

        # Hang it on c1...
        dg2 = Datagram.create_add_post_remove(dg)
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
        self.c2.send(Datagram.create_clear_post_remove())

        # Did the cancellation work?
        self.c2.close()
        self.__class__.c2 = self.new_connection()
        self.assertTrue(self.l1.expect_none())

    def test_ranges(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to range 1000-1999...
        dg = Datagram.create_add_range(1000, 1999)
        self.c1.send(dg)
        # Verify that l1 asks for the range as well...
        self.assertTrue(self.l1.expect(dg))

        # Send messages on a few channels on c2, see which ones c1 gets.
        def check_channels(channels):
            for channel, should_receive in channels:
                dg = Datagram.create([channel], 123456789, 5858)
                dg.add_uint16(channel) # For some semblance of uniqueness
                self.c2.send(dg)
                if should_receive:
                    self.assertTrue(self.c1.expect(dg))
                    self.assertTrue(self.c1.expect_none()) # No repeats!
                else:
                    self.assertTrue(self.c1.expect_none())
                # And, of course, l1 receives all of these:
                self.assertTrue(self.l1.expect(dg))
        check_channels([
            (500, False),
            (999, False),
            (1000, True),
            (1001, True),
            (1299, True),
            (1300, True),
            (1500, True),
            (1699, True),
            (1700, True),
            (1701, True),
            (1900, True),
            (1999, True),
            (2000, False),
            (2050, False),
            (2500, False)])

        # Ranged-subscriptions should still receive messages only once, even if
        # multiple channels are in the range.
        dg = Datagram.create([500, 1001, 1500], 0, 34)
        dg.add_string('test')
        self.c2.send(dg)
        self.assertTrue(self.c1.expect(dg))
        self.assertTrue(self.c1.expect_none()) # No repeats!
        self.assertTrue(self.l1.expect(dg))

        # Now let's "slice" the range.
        dg = Datagram.create_remove_range(1300, 1700)
        self.c1.send(dg)
        # l1 should request the slice upward
        self.assertTrue(self.l1.expect(dg))

        # And the slice should be gone:
        check_channels([
            (500, False),
            (999, False),
            (1000, True),
            (1001, True),
            (1299, True),
            (1300, False),
            (1500, False),
            (1699, False),
            (1700, False),
            (1701, True),
            (1900, True),
            (1999, True),
            (2000, False),
            (2050, False),
            (2500, False)])

        # How about adding a second range that overlaps?
        dg = Datagram.create_add_range(1900, 2100)
        self.c1.send(dg)
        # Verify that l1 asks for the range difference only...
        #self.assertTrue(self.l1.expect(Datagram.create_add_range(2000, 2100)))
        # NOTE: We actually want to recieve the entire range upstream
        self.assertTrue(self.l1.expect(dg))

        # Now the subscriptions should be updated:
        check_channels([
            (500, False),
            (999, False),
            (1000, True),
            (1001, True),
            (1299, True),
            (1300, False),
            (1500, False),
            (1699, False),
            (1700, False),
            (1701, True),
            (1900, True),
            (1999, True),
            (2000, True),
            (2050, True),
            (2500, False)])

        # Drop that first range...
        dg = Datagram.create_remove_range(1000, 1999)
        self.c1.send(dg)
        # And again, l1 should ask for difference only...
        # Difference #1: Drop 1000-1299
        self.l1.expect(Datagram.create_remove_range(1000, 1299))
        # Difference #2: Drop 1701-1999
        #self.l1.expect(Datagram.create_remove_range(1701, 1999))

        # Now see if only the second range is active...
        check_channels([
            (500, False),
            (999, False),
            (1000, False),
            (1001, False),
            (1999, False),
            (2000, True),
            (2050, True),
            (2500, False)])

        # Grand finale: Cut c1 and see if the remaining range dies.
        self.c1.close()
        self.__class__.c1 = self.new_connection()
        self.assertTrue(self.l1.expect(Datagram.create_remove_range(2000, 2100)))
        self.assertTrue(self.l1.expect_none())

if __name__ == '__main__':
    unittest.main()
