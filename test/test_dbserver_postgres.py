#!/usr/bin/env python2
import unittest, os, time, tempfile, subprocess, shutil
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
        type: postgresql
        host: 127.0.0.1
        port: 57023
        username: astron
        database: astron
""" % test_dc

class TestDatabaseServerPostgres(ProtocolTest, DatabaseBaseTests):
    @classmethod
    def setUpClass(cls):
        database_path = cls.temp = tempfile.gettempdir() + '/astron.postgresql'
        if not os.path.exists(database_path):
            os.makedirs(database_path);

        # Setup a postgresql instance owned by the local user
        os.system('initdb -D %s' % database_path)
        os.system('echo "unix_socket_directories = \'/tmp\'" >> %s/postgresql.conf' % database_path)
        cls.postgresd = subprocess.Popen(['postgres',
                                          '-D', database_path,
                                          '-h', '127.0.0.1', # bind address
                                          '-p', '57023'],    # bind port
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE)

        # Wait for postgres to start up:
        timeout = time.time() + 2.0
        while True:
            if time.time() > timeout:
                break

            try:
                pg_sock = socket(AF_INET, SOCK_STREAM)
                pg_sock.connect(('127.0.0.1', 57023))
            except error:
                time.sleep(0.2)
            else:
                pg_sock.close()
                break

        # Create a user and database in the instance
        os.system('createuser -p 57023 -h 127.0.0.1 --createdb astron')
        os.system('createdb -p 57023 -h 127.0.0.1 -U astron astron')

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
        time.sleep(0.25) # Wait for database to finish any operations
        cls.objects.send(Datagram.create_remove_range(DATABASE_PREFIX|1000000,
                                                      DATABASE_PREFIX|1000010))
        cls.objects.close()
        cls.conn.close()
        cls.daemon.stop()
        cls.postgresd.terminate()

        try:
            shutil.rmtree(cls.temp)
        except:
            pass


if __name__ == '__main__':
    unittest.main()
