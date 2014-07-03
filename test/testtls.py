import os

server_crt = os.path.abspath(os.path.join(os.path.dirname(__file__), 'tls/server.crt'))
server_key = os.path.abspath(os.path.join(os.path.dirname(__file__), 'tls/server.key'))
client_crt = os.path.abspath(os.path.join(os.path.dirname(__file__), 'tls/client.crt'))
client_key = os.path.abspath(os.path.join(os.path.dirname(__file__), 'tls/client.key'))
cert_auth = os.path.abspath(os.path.join(os.path.dirname(__file__), 'tls/cert_auth.crt'))
