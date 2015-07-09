#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *
from database.postgres import setup_postgres, teardown_postgres

class TestConfigDBPostgres(ConfigTest):
    @classmethod
    def setUpClass(cls):
        setup_postgres(cls)
        super(TestConfigDBPostgres, cls).setUpClass()

    @classmethod
    def tearDownClass(cls):
        super(TestConfigDBPostgres, cls).tearDownClass()
        teardown_postgres(cls)

    def test_postgres_good(self):
        config = """\
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
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_postgres_reserved_control(self):
        config = """\
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
                  backend:
                    type: soci
                    driver: postgresql
                    host: 127.0.0.1
                    port: 57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_postgres_invalid_generate(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 0
                    max: 1000010
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 1000000
                    max: 0
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_postgres_reserved_generate(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 444
                    max: 1000010
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 1000000
                    max: 555
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_postgres_boolean_broadcast(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: TRUE
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: False
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: 15012
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: soci
                    driver: postgresql
                    server: 127.0.0.1:57023
                    username: astron
                    database: astron
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

if __name__ == '__main__':
    unittest.main()
