#!/usr/bin/env python2
import unittest
from socket import *

from common import *
from testdc import *

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
        max: 1001000
      storage:
        type: bdb
        filename: main_database.db
""" % test_dc

class TestDatabaseServerBerkeley(unittest.TestCase, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)

if __name__ == '__main__':
    unittest.main()
