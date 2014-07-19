import os, time, tempfile, subprocess, shutil, time
from socket import socket, AF_INET, SOCK_STREAM

POSTGRESQL_SETUP_ERROR = """\
Could not find 'postgres' executable on path.
  -- postgresql server may not be installed
  -- ubuntu/debian does not put postgres on path:

       sudo ln -s /usr/lib/postgresql/*/bin/postgres /usr/bin
       sudo ln -s /usr/lib/postgresql/*/bin/initdb /usr/bin

"""

def setup_postgres(unittest):
    # Check for server command (for Ubuntu/Debian mostly)
    status = os.system('postgres --version')
    if status == 127:
        unittest.fail(POSTGRESQL_SETUP_ERROR)

    # Create temp folder to house database
    database_path = tempfile.gettempdir() + '/astron.postgresql'
    if not os.path.exists(database_path):
        os.makedirs(database_path);

    # Setup a postgresql instance owned by the local user
    os.system('initdb -D %s' % database_path)
    os.system('echo "unix_socket_directories = \'/tmp\'" >> %s/postgresql.conf' % database_path)

    # Start Postgres Server
    postgresd = subprocess.Popen(['postgres',
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
        except:
            time.sleep(0.2)
        else:
            pg_sock.close()
            break

    # Create a user and database in the instance
    os.system('createuser -p 57023 -h 127.0.0.1 --createdb astron')
    os.system('createdb -p 57023 -h 127.0.0.1 -U astron astron')

    # Set variables
    unittest.postgres_path = database_path
    unittest.postgresd = postgresd

def teardown_postgres(unittest):
    # Kill Server
    unittest.postgresd.terminate()

    # Remove temp files
    try:
        shutil.rmtree(unittest.postgres_path)
    except:
        pass
