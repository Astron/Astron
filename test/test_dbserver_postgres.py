#!/usr/bin/env python2
import unittest
from socket import *
from testdc import *
from common import *
from dbserver_base_tests import DatabaseBaseTests
from postgres_helper import setup_postgres, teardown_postgres

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
        type: postgresql
        host: 127.0.0.1
        port: 57023
        username: astron
        database: astron
""" % test_dc

class TestDatabaseServerPostgres(ProtocolTest, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        setup_postgres(cls)

        cls.daemon = Daemon(CONFIG)
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
        cls.objects.send(Datagram.create_remove_range(DATABASE_PREFIX|1000000,
                                                      DATABASE_PREFIX|1000010))
        cls.objects.close()
        cls.conn.close()
        cls.daemon.stop()

        teardown_postgres(cls)

if __name__ == '__main__':
    unittest.main()
