import tempfile, shutil

def setup_yamldb(unittest):
    unittest.yamldb_path = tempfile.mkdtemp(prefix = 'astron-', suffix = '.yamldb')

def teardown_yamldb(unittest):
    # Remove temp files
    try:
        shutil.rmtree(unittest.yamldb_path)
    except:
        pass