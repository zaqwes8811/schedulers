#!/usr/bin/env python3

import socket

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 4000        # The port used by the server

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

ss = "Value"
for i in range(1280):
	ss += "Value"+str(i)

print(len(ss))

sent = s.sendall(ss.encode())
print(sent)
data = s.recv(1024)

print('Received', repr(data))