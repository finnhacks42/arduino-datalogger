import socket
import sys

sock = socket.socket(socket.AF_INIT, socket.SOCK_STREAM)

server_address = ('localhost',80)
sock.bind(server_address)