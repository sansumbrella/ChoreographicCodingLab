#!/usr/loca/bin/python3
import socket
import json

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 9191))

def readHeader():
  header = s.recv(4)
  return int.from_bytes(header, byteorder="little")

def readBody(size):
  body = s.recv(size)
  string = body.decode("utf-8")
  return json.loads(string)


while True:
  # The server sends a header containing
  # a 4-byte integer telling us
  # how many bytes of json are coming
  size = readHeader()

  # Read the amount of data specified in header as json
  data = readBody(size)

  print(data["type"], data["points"][0])
