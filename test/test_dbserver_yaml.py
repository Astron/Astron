#!/usr/bin/env python2
import unittest, tempfile, shutil
from common.unittests import ProtocolTest
from common.dbserver import DBServerTestsuite
from common.astron import *
from common.dcfile import *
from database.yamldb import setup_yamldb, teardown_yamldb

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
        directory: %r
"""

class TestDatabaseServerYAML(ProtocolTest, DBServerTestsuite):
    @classmethod
    def setUpClass(cls):
        setup_yamldb(cls)
        cls.daemon = Daemon(CONFIG % (USE_THREADING, test_dc, cls.yamldb_path))
        cls.daemon.start()
        cls.conn = cls.connectToServer()
        cls.conn.s.settimeout(1.0) # Allow time for Astron<->filesystem operations.
        cls.objects = cls.connectToServer()
        cls.objects.send(Datagram.create_add_range(DATABASE_PREFIX|1000000,
                                                   DATABASE_PREFIX|1000010))

    @classmethod
    def tearDownClass(cls):
        cls.objects.send(Datagram.create_remove_range(DATABASE_PREFIX|1000000,
                                                      DATABASE_PREFIX|1000010))
        cls.objects.close()
        cls.conn.close()
        cls.daemon.stop()
        teardown_yamldb(cls)

if __name__ == '__main__':
    unittest.main()
