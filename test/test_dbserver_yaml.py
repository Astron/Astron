#!/usr/bin/env python2
import unittest
import os, time
from socket import *

from testdc import test_dc
from common import Daemon, MDConnection
from test_dbserver import DatabaseBaseTests

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

general:
    dc_files:
        - %r

roles:
    - type: database
      control: 777
      generate:
        min: 1000000
        max: 1000010
      engine:
        type: yaml
        foldername: unittest_db
""" % test_dc

class TestDatabaseServerYAML(unittest.TestCase, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        if not os.path.exists('unittest_db'):
            os.makedirs('unittest_db')

        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)

    @classmethod
    def tearDownClass(cls):
        time.sleep(0.25) # Wait for yaml db to finish writing to file
        cls.conn.close()
        cls.daemon.stop()

if __name__ == '__main__':
    unittest.main()
