#!/usr/bin/env python2
import unittest
import os
import time
import tempfile
import subprocess
from socket import *

from testdc import *
from common import *
from dbserver_base_tests import DatabaseBaseTests

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
        server: 127.0.0.1:57023
        database: test
""" % test_dc

class TestDatabaseServerMongo(ProtocolTest, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        tmppath = tempfile.gettempdir() + '/astron';
        if not os.path.exists(tmppath):
            os.makedirs(tmppath);
        dbpath = tempfile.mkdtemp(prefix='unittest.db-', dir=tmppath)

        cls.mongod = subprocess.Popen(['mongod',
                                       '--noauth', '--quiet',
                                       '--bind_ip', '127.0.0.1',
                                       '--port', '57023',
                                       '--dbpath', dbpath],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Wait for mongod to start up:
        while True:
            try:
                mongosock = socket(AF_INET, SOCK_STREAM)
                mongosock.connect(('127.0.0.1', 57023))
            except error:
                time.sleep(0.5)
            else:
                mongosock.close()
                break

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
        cls.daemon.stop()
        cls.mongod.terminate()

if __name__ == '__main__':
    unittest.main()
