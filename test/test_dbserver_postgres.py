#!/usr/bin/env python2
import unittest
from common.unittests import ProtocolTest
from common.dbserver import DBServerTestsuite
from common.astron import *
from common.dcfile import *
from database.postgres import setup_postgres, teardown_postgres

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
        type: soci
        driver: postgresql
        server: 127.0.0.1:57023
        username: astron
        database: astron
""" % (USE_THREADING, test_dc)

class TestDatabaseServerPostgres(ProtocolTest, DBServerTestsuite):
    @classmethod
    def setUpClass(cls):
        setup_postgres(cls)
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()
        cls.conn = cls.connectToServer()
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
        teardown_postgres(cls)

if __name__ == '__main__':
    unittest.main()
