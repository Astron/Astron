#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *
from database.yamldb import setup_yamldb, teardown_yamldb

class TestConfigDBYaml(ConfigTest):
    @classmethod
    def setUpClass(cls):
        setup_yamldb(cls)
        super(TestConfigDBYaml, cls).setUpClass()

    @classmethod
    def tearDownClass(cls):
        super(TestConfigDBYaml, cls).tearDownClass()
        teardown_yamldb(cls)

    def test_dbyaml_good(self):
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_dbyaml_reserved_control(self):
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_yamldb_invalid_generate(self):
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_yamldb_reserved_generate(self):
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_yamldb_boolean_broadcast(self):
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
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
                    type: yaml
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_yamldb_type_typo(self):
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
                    type: yam
                    directory: %r
            """ % (test_dc, self.yamldb_path)
        self.assertEquals(self.checkConfig(config), 'Invalid')

if __name__ == '__main__':
    unittest.main()
