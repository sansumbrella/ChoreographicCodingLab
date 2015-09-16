#!/usr/local/bin/python3

from urllib import request
import json

def load_json(url):
  f = request.urlopen(url).read().decode("utf-8")
  return json.loads(f)

def load_all_streams(channel_uuid):
  url = ("http://api.piecemeta.com/channels/%s/streams.json") % channel_uuid
  js = load_json(url)
  for stream in js:
    uuid = stream["uuid"]
    print(stream["title"])
    print(uuid)
    stream_json = load_stream(uuid)
    yield stream_json

def load_stream(uuid):
  url = ("http://api.piecemeta.com/streams/%s.json?skip=4") % uuid
  contents = request.urlopen(url).read().decode("utf-8")
  return json.loads(contents)

def make_filename(stream):
  return stream["group"] + "-" + stream["title"] + ".json"

def write_all_streams(channel_uuid):
  streams = []
  for f in load_all_streams(channel_uuid):
    streams.append(f)
  grouped_streams = [f for f in streams if "group" in f]

  for stream in grouped_streams:
    name = make_filename(stream)
    f = open(name, "w")
    f.write(json.dumps(stream))

channel_uuid = "73f62987-1243-4d8d-9909-dc61c3b16a60"
