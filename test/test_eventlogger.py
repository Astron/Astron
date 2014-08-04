#!/usr/bin/env python2
import unittest, time, socket, os, json, tempfile
from common.astron import *
from common.dcfile import *

NETWORK_WAIT = 0.1 # seconds
NETWORK_ADDR = ('127.0.0.1', 19090)
MD_ADDR = ('127.0.0.1',  57123)

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

roles:
    - type: eventlogger
      bind: 127.0.0.1:19090
      output: %s
"""

STANDARD_EVENT = '\x82\xa3bar\xa3baz\xa4type\xa3foo' # MessagePack formatted event
NONSTANDARD_MSGPACK = '{'

class TestEventLogger(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logHandle, cls.log_file = tempfile.mkstemp(prefix = 'astron-', suffix = '.log')
        os.close(logHandle)

        cls.daemon = Daemon(CONFIG % cls.log_file)
        cls.daemon.start()

        cls.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        cls.mdsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        cls.mdsocket.connect(MD_ADDR)

    @classmethod
    def tearDownClass(cls):
        cls.socket.close()
        cls.mdsocket.close()
        cls.daemon.stop()
        time.sleep(1) # give time for the daemon to close so windows can delete the log
        if cls.log_file is not None:
            os.remove(cls.log_file)

    def numLinesLog(self):
        log = open(self.log_file, "r")
        contents = log.read()
        return len(contents.split("\n"))

    def test_writesToLog(self):
        numLines1 = self.numLinesLog()

        self.socket.sendto(STANDARD_EVENT, NETWORK_ADDR)
        time.sleep(NETWORK_WAIT) # allow network time

        self.assertEqual(self.numLinesLog(), numLines1 + 1) # exactly one line is added

    def test_ignoresNonJSON(self):
        numLines1 = self.numLinesLog()

        self.socket.sendto(NONSTANDARD_MSGPACK, NETWORK_ADDR)
        time.sleep(NETWORK_WAIT) # allow network time

        self.assertEqual(self.numLinesLog(), numLines1) # log is unchanged

    def test_ignoresNonMsgpack(self):
        numLines1 = self.numLinesLog()

        self.socket.sendto("123", NETWORK_ADDR)
        time.sleep(NETWORK_WAIT) # allow network time

        self.assertEqual(self.numLinesLog(), numLines1) # log is unchanged

    def lastLineCheck(self):
        log = open(self.log_file, "r")
        contents = log.read().split("\n")
        lastLine = contents[len(contents) - 2]

        decodedJSON = json.loads(lastLine)

        self.assertEqual(decodedJSON["type"], "foo")
        self.assertEqual(decodedJSON["bar"], "baz")
        self.assertIn("_time", decodedJSON)

    def test_logMessageFormatted(self):
        self.socket.sendto(STANDARD_EVENT, NETWORK_ADDR)
        time.sleep(NETWORK_WAIT) # allow network time

        self.lastLineCheck()

    def test_messageDirectorLogging(self):
        dg = Datagram.create_control()
        dg.add_uint16(CONTROL_LOG_MESSAGE)
        dg.add_blob(STANDARD_EVENT)

        self.mdsocket.send(dg.get_data())
        time.sleep(NETWORK_WAIT)

        self.lastLineCheck()


if __name__ == '__main__':
    unittest.main()
