#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *

class TestConfigDBSS(ConfigTest):
    def test_dbss_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 9999
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_dbss_invalid_attr(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  pewpew: "q.q"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbss_invalid_database(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 0
                  ranges:
                      - min: 9000
                        max: 9999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbss_reserved_database(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 200
                  ranges:
                      - min: 9000
                        max: 9999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbss_invalid_range(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 0
                        max: 9999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 0
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_dbss_reserved_range(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 201
                        max: 9999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 206
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

if __name__ == '__main__':
    unittest.main()
