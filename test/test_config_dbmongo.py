#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *
from database.mongo import setup_mongo, teardown_mongo

class TestConfigDBMongo(ConfigTest):
    @classmethod
    def setUpClass(cls):
        setup_mongo(cls)
        super(TestConfigDBMongo, cls).setUpClass()

    @classmethod
    def tearDownClass(cls):
        super(TestConfigDBMongo, cls).tearDownClass()
        teardown_mongo(cls)

    def test_dbmongo_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: false
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_dbmongo_reserved_control(self):
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
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbmongo_invalid_generate(self):
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
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
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
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbmongo_reserved_generate(self):
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
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
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
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbmongo_boolean_broadcast(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: FALSE
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
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
                  broadcast: pizza
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: mongodb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbmongo_type_typo(self):
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
                    max: 1000010
                  backend:
                    type: mongdb
                    server: mongodb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbmongo_bad_uri(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: false
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: mongodb
                    server: baddb://127.0.0.1:57023/test
            """ % (test_dc)
        self.assertEquals(self.checkConfig(config), 'Invalid')

if __name__ == '__main__':
    unittest.main()
