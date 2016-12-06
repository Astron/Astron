#!/usr/bin/env python2
import unittest
from common.unittests import ProtocolTest
from common.dbserver import DBServerTestsuite
from common.astron import *
from common.dcfile import *
from database.mongo import setup_mongo, teardown_mongo

CONFIG = """\
messagedirector:
    bind: 127.0.0.1:57123

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
        type: mongodb
        server: mongodb://127.0.0.1:57023/test
""" % test_dc

class TestDatabaseServerMongo(ProtocolTest, DBServerTestsuite):
    @classmethod
    def setUpClass(cls):
        setup_mongo(cls)
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()
        cls.conn = cls.connectToServer()
        cls.conn.s.settimeout(1.0) # Allow time for Astron<->MongoDB communication.
        cls.objects = cls.connectToServer()
        cls.objects.send(Datagram.create_add_range(DATABASE_PREFIX|1000000,
                                                   DATABASE_PREFIX|1000010))

    @classmethod
    def tearDownClass(cls):
        cls.objects.send(Datagram.create_remove_range(DATABASE_PREFIX|1000000,
                                                      DATABASE_PREFIX|1000010))
        cls.objects.close()
        cls.daemon.stop()
        cls.mongod.terminate()
        teardown_mongo(cls)

if __name__ == '__main__':
    unittest.main()
