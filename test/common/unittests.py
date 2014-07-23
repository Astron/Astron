import unittest, subprocess, tempfile, os, threading
from socket import socket, AF_INET, SOCK_STREAM
from astron import *

class ConfigTest(unittest.TestCase):
    class ConfigRunner(object):
        DAEMON_PATH = './astrond'

        def __init__(self, config):
            self.config = config
            self.process = None

        def run(self, timeout):
            def target():
                self.process = subprocess.Popen([self.DAEMON_PATH, self.config])
                self.process.communicate()

            thread = threading.Thread(target=target)
            thread.start()

            thread.join(timeout)
            if thread.is_alive():
                self.process.terminate()
                thread.join()
                return 'Valid'
            return 'Invalid'

    @classmethod
    def writeConfig(cls, config):
        f = open(cls.config_path, "w")
        f.write(config)
        f.close()

    @classmethod
    def checkConfig(cls, config, timeout = 2):
        cls.writeConfig(config)
        return cls.test_runner.run(timeout)

    @classmethod
    def setUpClass(cls):
        file_handle, cls.config_path = tempfile.mkstemp(prefix = 'astron-', suffix = '.cfg.yaml')
        os.close(file_handle)

        cls.test_runner = ConfigTest.ConfigRunner(cls.config_path)

    @classmethod
    def tearDownClass(cls):
        if cls.config_path is not None:
            os.remove(cls.config_path)

class ProtocolTest(unittest.TestCase):
    @classmethod
    def connectToServer(cls, addr = '127.0.0.1', port = 57123):
        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect((addr, port))
        return MDConnection(sock)

    def writeUnexpectedAndFail(self, received):
        testName = self.__class__.__name__
        f = open("%s-received.bin" % testName, "wb")
        f.write(received.get_data())
        f.close()
        self.fail("Received datagram when expecting none.\n" +
                  "\tWritten to \"%s-received.bin\"." % testName)

    def writeDatagramsAndFail(self, expected, received):
        testName = self.__class__.__name__
        f = open("%s-expected.bin" % testName, "wb")
        f.write(expected.get_data())
        f.close()
        f = open("%s-received.bin" % testName, "wb")
        f.write(received.get_data())
        f.close()
        self.fail("Received datagram doesn't match expected.\n" +
                  "\tWritten to \"%s-{expected,received}.bin\"." % testName)

    def assertDatagramsEqual(self, expected, received, isClient = False):
        lhs = DatagramIterator(expected)
        rhs = DatagramIterator(received)

        if isClient:
            expectedMsgtype = lhs.read_uint16()
            receivedMsgtype = rhs.read_uint16()
            self.assertEquals(expectedMsgtype, receivedMsgtype)

            if not received.equals(expected):
                self.writeDatagramsAndFail(expected, received)

        else:
            numChannelsExpected = lhs.read_uint8()
            numChannelsReceived = rhs.read_uint8()
            self.assertEquals(numChannelsExpected, numChannelsReceived)

            expectedRecipients = expected.get_channels()
            receivedRecipients = received.get_channels()
            self.assertEquals(expectedRecipients, receivedRecipients)

            lhs.seek(1 + CHANNEL_SIZE_BYTES * numChannelsExpected)
            rhs.seek(1 + CHANNEL_SIZE_BYTES * numChannelsReceived)

            if expectedRecipients != set([CONTROL_CHANNEL]):
                # If we aren't a control datagram, check the sender
                expectedSender = lhs.read_channel()
                receivedSender = rhs.read_channel()
                self.assertEquals(expectedSender, receivedSender)

            expectedMsgtype = lhs.read_uint16()
            receivedMsgtype = rhs.read_uint16()
            self.assertEquals(expectedMsgtype, receivedMsgtype)

            if not received.matches(expected):
                self.writeDatagramsAndFail(expected, received)

    def expect(self, conn, expected, isClient = False):
        received = conn.recv_maybe()
        if received is None:
            self.fail("No datagram received.")
        self.assertDatagramsEqual(expected, received, isClient)

    def expectMany(self, conn, datagrams, ignoreExtra = False, isClient = False):
        datagrams = list(datagrams) # We're going to be doing datagrams.remove()
        recvs = []

        numRecvd = 0
        numMatch = 0
        numExpct = len(datagrams)
        while datagrams:
            received = conn.recv_maybe()
            if received is None:
                if numMatch == 0:
                    self.fail("Received %d datagrams, but expected %d." % (numRecvd, numExpct))
                else:
                    error = "Received %d datagrams, of which %d matched, but expected %d."
                    error += "\n  Received msgtypes: ( "
                    for dg in recvs:
                        error += "%s " % dg.get_msgtype()
                    error = error % (numRecvd, numMatch, numExpct)
                    error += ")"
                    self.fail(error)
            numRecvd += 1

            for datagram in datagrams:
                if (isClient and received.equals(datagram)) or received.matches(datagram):
                    recvs.append(datagram)
                    datagrams.remove(datagram)
                    numMatch += 1
                    break
            else:
                if not ignoreExtra:
                    best = None
                    for datagram in datagrams:
                        # Try to find the most similar datagram
                        if datagram.get_channels() == received.get_channels():
                            self.assertDatagramsEqual(datagram, received, isClient)
                            break
                        elif datagram.get_size() == received.get_size() and best is None:
                            best = datagram
                    else:
                        if best is not None:
                            self.assertDatagramsEqual(best, received, isClient)
                        else:
                            self.assertDatagramsEqual(datagrams[0], received, isClient)
                    # This should always fail, but it produces more useful
                    # debugging output.  Lets guarantee that it fails for fun.
                    self.fail("Testsuite implementation error.")

    def expectNone(self, conn):
        received = conn.recv_maybe()
        if received is not None:
            self.writeUnexpectedAndFail(received)