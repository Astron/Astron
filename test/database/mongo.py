import os, time, tempfile, subprocess, shutil
from socket import socket, AF_INET, SOCK_STREAM

def setup_mongo(unittest):
    # Create temp folder to house database
    mongo_path = tempfile.mkdtemp(prefix = 'astron-', suffix = '.mongodb')

    # Setup an unpriveledged mongo instance
    mongod = subprocess.Popen(['mongod',
                               '--noauth', '--quiet',
                               '--nojournal', '--noprealloc',
                               '--bind_ip', '127.0.0.1',
                               '--port', '57023',
                               '--dbpath', mongo_path],
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE)

    # Wait for mongod to start up:
    while True:
        try:
            mongo_sock = socket(AF_INET, SOCK_STREAM)
            mongo_sock.connect(('127.0.0.1', 57023))
        except:
            time.sleep(0.5)
        else:
            mongo_sock.close()
            break

    # Set variables
    unittest.mongo_path = mongo_path
    unittest.mongod = mongod

def teardown_mongo(unittest):
    # Kill Server
    unittest.mongod.terminate()

    # Remove temp files
    try:
        shutil.rmtree(unittest.mongo_path)
    except:
        pass
