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
        type: bdb
        filename: main_database.db
""" % test_dc
