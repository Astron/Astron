#!/usr/bin/env python2
import unittest

'''
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
      storage:
        type: mongodb
        connection: mongodb://astron_test:sudowoodo@localhost
        database: astron_test
""" % test_dc

class TestDatabaseServerMongo(ProtocolTest, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        cls.daemon = Daemon(CONFIG)
        cls.daemon.start()

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect(('127.0.0.1', 57123))
        cls.conn = MDConnection(sock)

if __name__ == '__main__':
    unittest.main()
'''
