import os, time, tempfile, subprocess, shutil
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
    status = 0
    try:
        status = os.system('postgres --version')
    except:
        pass

    if status == 127:
        unittest.fail(POSTGRESQL_SETUP_ERROR)

    # Create temp folder to house database
    postgres_path = tempfile.mkdtemp(prefix = 'astron-', suffix = '.postgresql')

    # Setup a postgresql instance owned by the local user
    os.system('initdb -D %s' % postgres_path)
    os.system('echo "unix_socket_directories = \'/tmp\'" >> %s/postgresql.conf' % postgres_path)

    # Start Postgres Server
    postgresd = subprocess.Popen(['postgres',
                                  '-D', postgres_path,
                                  '-h', '127.0.0.1', # bind address
                                  '-p', '57023'],    # bind port
                                  stdout = subprocess.PIPE,
                                  stderr = subprocess.PIPE)

    # Postgresql is rude and happily accepts connections before its ready to
    #     deal with them. So we'll sleep a little just to make sure its ready.
    time.sleep(0.5)

    # Create a user and database in the instance
    os.system('createuser -p 57023 -h 127.0.0.1 --superuser --createdb astron')
    os.system('createdb -p 57023 -h 127.0.0.1 --username=astron astron')

    # Set variables
    unittest.postgres_path = postgres_path
    unittest.postgresd = postgresd

def teardown_postgres(unittest):
    # Kill Server
    unittest.postgresd.terminate()
    unittest.postgresd.wait()

    # Remove temp files
    try:
        shutil.rmtree(unittest.postgres_path)
    except:
        pass
