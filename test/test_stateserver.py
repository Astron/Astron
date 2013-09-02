#!/usr/bin/env python2
import unittest
from socket import *

from common import *
from test_dc import *

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

general:
    dc_files:
        - %r

roles:
    - type: stateserver
      control: 100
""" % test_dc

class TestStateServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        cls.c = MDConnection(c)

    @classmethod
    def tearDownClass(cls):
        cls.c.close()
        cls.daemon.stop()

    def test_create_destroy(self):
        self.c.flush()
        self.c.send(Datagram.create_add_channel(5000<<32|1500))

        # Create a DistributedTestObject1...
        dg = Datagram.create([100], 5, STATESERVER_OBJECT_GENERATE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(101000000) # ID
        dg.add_uint32(6789) # setRequired1
        self.c.send(dg)

        # The object should announce its entry to the zone-channel...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
        dg.add_uint32(5000) # Parent
        dg.add_uint32(1500) # Zone
        dg.add_uint16(DistributedTestObject1)
        dg.add_uint32(101000000) # ID
        dg.add_uint32(6789) # setRequired1
        self.assertTrue(self.c.expect(dg))

        # Destroy our object...
        dg = Datagram.create([101000000], 5, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        self.c.send(dg)

        # Object should announce its disappearance...
        dg = Datagram.create([5000<<32|1500], 101000000, STATESERVER_OBJECT_DELETE_RAM)
        dg.add_uint32(101000000)
        self.assertTrue(self.c.expect(dg))

        # We're done here...
        self.c.send(Datagram.create_remove_channel(5000<<32|1500))


if __name__ == '__main__':
    unittest.main()
