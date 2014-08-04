#!/usr/bin/env python2
import unittest
from socket import *

from common.unittests import ProtocolTest
from common.astron import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    connect: 127.0.0.1:57124
"""

class TestMessageDirector(ProtocolTest):
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

        cls.c1 = cls.connectToServer()
        cls.c2 = cls.connectToServer()

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
        self.expect(self.l1, dg)

    def test_subscribe(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to a channel...
        dg = Datagram.create_add_channel(12345654321)
        self.c1.send(dg)
        self.expectNone(self.c1)
        # Make sure the MD subscribes to its parent.
        self.expect(self.l1, dg)

        # Send a test datagram on second connection...
        dg = Datagram()
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.expect(self.c1, dg)
        # MD will, of course, relay this datagram upward.
        self.expect(self.l1, dg)

        # Subscribe on the second connection...
        dg = Datagram.create_add_channel(12345654321)
        self.c2.send(dg)
        self.expectNone(self.c2)
        # MD should not ask for the channel a second time.
        self.expectNone(self.l1)

        # Send a test datagram on first connection...
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c1.send(dg)
        self.expect(self.c2, dg) # Should be relayed to second.
        self.expect(self.l1, dg) # Should be sent upward.
        #self.expectNone(self.c1) # Should NOT be echoed back.

        # Unsubscribe on the first connection...
        dg = Datagram.create_remove_channel(12345654321)
        self.c1.send(dg)
        self.expectNone(self.c1)
        self.expectNone(self.c2)
        # MD should NOT unsubscribe from parent!
        self.expectNone(self.l1)

        # Send another test datagram on second connection...
        dg = Datagram.create([12345654321], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.c2.send(dg)
        self.expect(self.l1, dg) # Should be sent upward.
        self.expectNone(self.c2) # Should NOT be relayed.
        self.expectNone(self.c1) # Should NOT be echoed back.

        # Abandon the second connection, which should auto-unsubscribe it.
        self.c2.close()
        self.__class__.c2 = self.connectToServer()
        self.expectNone(self.c1)
        # MD should unsubscribe from parent.
        self.expect(self.l1, Datagram.create_remove_channel(12345654321))

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
        self.expect(self.c1, dg)
        self.expect(self.c2, dg)

        # A datagram to channels 1111 and 3333 should be delievered to both.
        dg = Datagram.create([1111, 3333], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.expect(self.c1, dg)
        self.expect(self.c2, dg)

        # A datagram should only be delivered once if multiple channels match.
        dg = Datagram.create([1111, 2222], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.expect(self.c1, dg)
        self.expectNone(self.c1)
        self.expect(self.c2, dg)

        # Let's try something really absurd:
        dg = Datagram.create([1111, 2222, 3333, 1111, 1111,
                              2222, 3333, 3333, 2222], 0, 1234)
        dg.add_uint32(0xDEADBEEF)
        self.l1.send(dg)
        self.expect(self.c1, dg)
        self.expectNone(self.c1)
        self.expect(self.c2, dg)
        self.expectNone(self.c2)

        # And, of course, sending this monstrosity on c1 should result in it
        # showing up on c2 and l1 once only; no echo back on c1.
        self.c1.send(dg)
        self.expect(self.l1, dg)
        self.expectNone(self.l1)
        self.expect(self.c2, dg)
        self.expectNone(self.c2)
        self.expectNone(self.c1)

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
        dg_pr = Datagram.create([555444333], 0, 4321)
        dg_pr.add_string('Testing...')

        # Hang it on c1...
        dg_add_pr = Datagram.create_add_post_remove(171717, dg_pr)
        self.c1.send(dg_add_pr)

        # Expect post remove to be pre-routed upstream
        self.expect(self.l1, dg_add_pr)

        # Verify nothing else is happening yet...
        self.expectNone(self.l1)
        self.expectNone(self.c1)
        self.expectNone(self.c2)

        # Reconnect c1 and see if dg gets sent.
        self.c1.close()
        self.__class__.c1 = self.connectToServer()

        # The upstream should receive the post remove and also receive a clear_post_removes
        dg_clear_prs = Datagram.create_clear_post_removes(171717)
        self.expectMany(self.l1, [dg_pr, dg_clear_prs])

        # Reconnect c1, the message shouldn't be sent again
        self.c1.close()
        self.__class__.c1 = self.connectToServer()
        self.expectNone(self.l1)

        # Hang dg as a post-remove for c2...
        dg_add_pr = Datagram.create_add_post_remove(181818, dg_pr)
        self.c2.send(dg_add_pr)
        self.expect(self.l1, dg_add_pr)

        # Wait, nevermind...
        dg_clear_prs = Datagram.create_clear_post_removes(181818)
        self.c2.send(dg_clear_prs)
        self.expect(self.l1, dg_clear_prs)

        # Did the cancellation work?
        self.c2.close()
        self.__class__.c2 = self.connectToServer()
        self.expectNone(self.l1)

        # Try hanging multiple post-removes on c1
        dg_add_pr = Datagram.create_add_post_remove(191919, dg_pr)
        dg_pr2 = Datagram.create([987987987], 0, 6959)
        dg_add_pr2 = Datagram.create_add_post_remove(191919, dg_pr2)
        dg_pr3 = Datagram.create([986666687], 0, 1252)
        dg_add_pr3 = Datagram.create_add_post_remove(202020, dg_pr3)
        self.c1.send(dg_add_pr)
        self.c1.send(dg_add_pr2)
        self.c1.send(dg_add_pr3)

        # Expect post removes to be pre-routed upstream
        self.expectMany(self.l1, [dg_add_pr, dg_add_pr2, dg_add_pr3])

        # After adding three, we don't want to see anything "pushed" or the like
        self.expectNone(self.l1)
        self.expectNone(self.c1)
        self.expectNone(self.c2)

        # Reconnect c1 and see if both datagrams gets sent ...
        self.c1.close()
        self.__class__.c1 = self.connectToServer()

        # ... expecting the post removes ...
        expected = [dg_pr, dg_pr2, dg_pr3]
        # ... and a clear for each channel ...
        dg_clear_prs = Datagram.create_clear_post_removes(191919)
        dg_clear_prs2 = Datagram.create_clear_post_removes(202020)
        expected += [dg_clear_prs, dg_clear_prs2]
        # ... exactly 2 Post Removes, and 3 Clears ...
        self.expectMany(self.l1, expected)
        # ... and no more messages (duplicates or otherwise)
        self.expectNone(self.l1)

    def test_ranges(self):
        self.l1.flush()
        self.c1.flush()
        self.c2.flush()

        # Subscribe to range 1000-1999...
        dg = Datagram.create_add_range(1000, 1999)
        self.c1.send(dg)
        # Verify that l1 asks for the range as well...
        self.expect(self.l1, dg)

        # Send messages on a few channels on c2, see which ones c1 gets.
        def check_channels(channels):
            for channel, should_receive in channels:
                dg = Datagram.create([channel], 123456789, 5858)
                dg.add_uint16(channel) # For some semblance of uniqueness

                self.c2.send(dg)

                if should_receive:
                    self.expect(self.c1, dg)
                    self.expectNone(self.c1) # No repeats!
                else:
                    self.expectNone(self.c1)

                # And, of course, l1 receives all of these:
                self.expect(self.l1, dg)

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
        self.expect(self.c1, dg)
        self.expectNone(self.c1) # No repeats!
        self.expect(self.l1, dg)

        # Now let's "slice" the range.
        dg = Datagram.create_remove_range(1300, 1700)
        self.c1.send(dg)
        # l1 should request the slice upward
        self.expect(self.l1, dg)

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
        # Verify that md asks for the entire range from l1...
        self.expect(self.l1, dg)

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

        # -- See comments after block --
        ## And again, l1 should ask for difference only...
        #expected = []
        ## Difference #1: Drop 1000-1299
        #expected.append(Datagram.create_remove_range(1000, 1299))
        ## Difference #2: Drop 1701-1999
        #expected.append(Datagram.create_remove_range(1701, 1999))
        #self.expectMany(self.l1, expected)

        # In this case because there are no remaining subscriptions in the
        # entire removed range, it is more efficient for the network, CPU,
        # and memory to just forward the entire range.
        dg = Datagram.create_remove_range(1000, 1999)
        self.expect(self.l1, dg)

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
        self.c1 = self.connectToServer()
        self.expect(self.l1, Datagram.create_remove_range(2000, 2100))
        self.expectNone(self.l1)

        # ... but we lied! Now there are some more tests

        # Lets add a new range to play with
        dg = Datagram.create_add_range(3000, 5000)
        self.c1.send(dg)
        self.expect(self.l1, dg)
        check_channels([
            (2499, False),
            (2501, False),
            (2999, False),
            (3000, True),
            (4000, True),
            (4372, True),
            (5000, True),
            (5001, False),
            (5109, False)])

        # Test removing a range that intersects with the front part
        dg = Datagram.create_remove_range(2950, 3043)
        self.c1.send(dg)
        # --> and expect only the subscribed part to be removed
        dg = Datagram.create_remove_range(3000, 3043)
        self.expect(self.l1, dg)
        check_channels([
            (2913, False),
            (2999, False),
            (3000, False),
            (3043, False),
            (3044, True),
            (4000, True),
            (4372, True),
            (5000, True),
            (5001, False),
            (5109, False)])

        # Test removing a range that intersects with the end part
        dg = Datagram.create_remove_range(4763, 6000)
        self.c1.send(dg)
        # --> and expect only the subscribed part to be removed
        dg = Datagram.create_remove_range(4763, 5000)
        self.expect(self.l1, dg)
        check_channels([
            (3000, False),
            (3043, False),
            (3044, True),
            (4000, True),
            (4372, True),
            (4762, True),
            (4763, False),
            (5000, False),
            (5001, False),
            (5109, False)])

        # Now remove some from the middle again so we can test weird intersections
        dg = Datagram.create_remove_range(3951, 4049)
        self.c1.send(dg)
        # The entire range is subscribed, so it should all be unsubscribed
        self.expect(self.l1, dg)
        check_channels([
            (3043, False),
            (3044, True),
            (3802, True),
            (3950, True),
            (3951, False),
            (4049, False),
            (4050, True),
            (4133, True),
            (4762, True),
            (4763, False)])

        # Ok... remove an intersection from the lower half of the upper range
        dg = Datagram.create_remove_range(4030, 4070)
        self.c1.send(dg)
        # N.B. Its worth considering which of the following behaviors is preferred,
        #      the first is a lot more work for the current MD, but may reduce the
        #      work for other MDs. Consider performance testing both implementations.
        ## --> and expect only the subscribed part to be removed
        #self.expect(self.l1, Datagram.create_remove_range(4050, 4070))
        self.expect(self.l1, dg)
        check_channels([
            (3043, False),
            (3044, True),
            (3802, True),
            (3950, True),
            (3951, False),
            (4070, False),
            (4071, True),
            (4133, True),
            (4762, True),
            (4763, False)])

        # Now remove an intersection from the upper half of the lower range
        dg = Datagram.create_remove_range(3891, 4040)
        self.c1.send(dg)
        # N.B. Its worth considering which of the following behaviors is preferred,
        #      the first is a lot more work for the current MD, but may reduce the
        #      work for other MDs. Consider performance testing both implementations.
        ## --> and expect only the subscribed part to be removed
        #self.expect(self.l1, Datagram.create_remove_range(3891, 3950))
        self.expect(self.l1, dg)
        check_channels([
            (3043, False),
            (3044, True),
            (3672, True),
            (3890, True),
            (3891, False),
            (3893, False),
            (4070, False),
            (4071, True),
            (4762, True),
            (4763, False)])

        # Now lets intersect part of both the upper and lower range
        dg = Datagram.create_remove_range(3700, 4200)
        self.c1.send(dg)
        # N.B. Its worth considering which of the following behaviors is preferred,
        #      the first is a lot more work for the current MD, but may reduce the
        #      work for other MDs. Consider performance testing both implementations.
        #      Additionally the first requires twice as much network traffice, but
        #      it is still relatively small on a relatively infrequent operation.
        ## --> and expect only the subscribed parts to be removed
        #expected = []
        #expected.append(Datagram.create_remove_range(3700, 3890))
        #expected.append(Datagram.create_remove_range(4070, 4200))
        #self.expectMany(self.l1, expected)
        self.expect(self.l1, dg)
        check_channels([
            (3043, False),
            (3044, True),
            (3699, True),
            (3700, False),
            (4200, False),
            (4201, True),
            (4762, True),
            (4763, False)])

        # Now lets subscribe our 2nd client to an intersecting range
        dg = Datagram.create_add_range(3500, 4500)
        self.c2.send(dg)
        self.expect(self.l1, dg)

        # Now remove an upper part of the lower range that is contained within c2's range
        dg = Datagram.create_remove_range(3650, 3800)
        self.c1.send(dg)
        # We shouldn't get a remove for this upstream, because the 2nd client is still interested
        self.expectNone(self.l1)
        check_channels([
            # Lower range
            (3043, False),
            (3044, True), # lower bound
            (3333, True),
            (3480, True),
            (3499, True),
            (3500, True),
            (3649, True), # upper bound
            (3650, False),

            (3787, False),
            (4000, False),

            # Upper range
            (4200, False),
            (4201, True)]) # lower bound

        # Now remove part of the lower range that contains just the lower bound of c2's range,
        # but not the upper bound of the lower range.
        dg = Datagram.create_remove_range(3475, 3525)
        self.c1.send(dg)
        # We should expect to receive only the portion of the range which is outside c2's range
        self.expect(self.l1, Datagram.create_remove_range(3475, 3499))
        check_channels([
            # Lower range
            (3043, False),
            (3044, True), # lower bound
            (3474, True), # upper bound
            (3475, False),

            (3482, False),
            (3499, False),

            # Mid range
            (3525, False),
            (3526, True), # Lower bound
            (3600, True),
            (3649, True), # upper bound
            (3650, False),

            # Upper range
            (4200, False),
            (4201, True)]) # lower bound

        # Now remove a range from c2 which contains the mid-range's upper bound
        # and the upper-range's lower bound.
        dg = Datagram.create_remove_range(3620, 4300)
        self.c2.send(dg)
        # We should expect to recieve only the portion of c2 which is between the two ranges
        self.expect(self.l1, Datagram.create_remove_range(3650, 4200))
        check_channels([
            # Lower range
            (3474, True), # upper bound
            (3475, False),

            # Mid range
            (3525, False),
            (3526, True), # Lower bound
            (3649, True), # upper bound
            (3650, False),

            # Upper range
            (4200, False),
            (4201, True), # lower bound
            (4762, True), # upper bound
            (4763, False)])

        # Cut c2 and watch the part die that is not in c1
        self.c2.close()
        self.c2 = self.connectToServer()
        self.expect(self.l1, Datagram.create_remove_range(3500, 3525))
        self.expectNone(self.l1)

        # Now add c2 such that it contains all the c1 ranges
        dg = Datagram.create_add_range(1000, 5000)
        self.c2.send(dg)
        self.expect(self.l1, dg)

        # Then remove that range and see all the inbetween parts die
        self.c2.send(Datagram.create_remove_range(1000, 5000))
        expected = []
        expected.append(Datagram.create_remove_range(1000, 3043))
        expected.append(Datagram.create_remove_range(3475, 3525))
        expected.append(Datagram.create_remove_range(3650, 4200))
        expected.append(Datagram.create_remove_range(4763, 5000))
        self.expectMany(self.l1, expected)
        self.expectNone(self.l1)

        # Cleanup
        self.c1.close()
        self.c2.close()
        self.__class__.c1 = self.connectToServer()
        self.__class__.c2 = self.connectToServer()
        self.l1.flush()

if __name__ == '__main__':
    unittest.main()
