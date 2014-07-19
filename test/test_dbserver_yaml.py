#!/usr/bin/env python2
import unittest, os, time, tempfile, shutil
from socket import *

from testdc import *
from common import *
from dbserver_base_tests import DatabaseBaseTests

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123
    threaded: %s

general:
    dc_files:
        - %r

roles:
    - type: database
      control: 75757
      broadcast: true
      generate:
        min: 1000000
        max: 1000010
      backend:
        type: yaml
        foldername: %r
"""

class TestDatabaseServerYAML(ProtocolTest, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        tmppath = tempfile.gettempdir() + '/astron';
        if not os.path.exists(tmppath):
            os.makedirs(tmppath);
        dbpath = tempfile.mkdtemp(prefix='unittest.db-', dir=tmppath)

        cls.daemon = Daemon(CONFIG % (USE_THREADING, test_dc, dbpath))
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.objects = MDConnection(sock)
        cls.objects.send(Datagram.create_add_range(DATABASE_PREFIX|1000000,
                                                   DATABASE_PREFIX|1000010))

    @classmethod
    def tearDownClass(cls):
        time.sleep(0.25) # Wait for yaml db to finish writing to file
        cls.objects.send(Datagram.create_remove_range(DATABASE_PREFIX|1000000,
                                                      DATABASE_PREFIX|1000010))
        cls.objects.close()
        cls.conn.close()
        cls.daemon.stop()

if __name__ == '__main__':
    unittest.main()
