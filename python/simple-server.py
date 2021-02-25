#!/usr/bin/env python
# -*- coding:utf-8 -*-

import argparse
import json
import sys
import pathlib
import urllib

from http.server import BaseHTTPRequestHandler, HTTPServer
from http import HTTPStatus

DATA_JSON='data.json'
ERR_JSON={'status':'none'}

def loadJson(file):
	try:
		with open(file, mode='r', encoding='utf-8') as f:
			text = f.read()
			return json.loads(text)
	except:
		return {}

def saveJson(file, js):
	with open(file, mode='w', encoding='utf-8') as f:
		text = f.write(json.dumps(js))

def getContentType(suffix):
	tbl = {
		'js' : 'text/javascript',
		'html' : 'text/html',
	}
	return tbl.get(suffix[1:], 'text/plain')

def returnFile(self, path):
	contentType = getContentType(path.suffix)
	print('returnFile name=%s contentType=%s' % (path.name, contentType))
	try:
		with open(path.name, mode='r', encoding='utf-8') as f:
			text = f.read()
	except:
		text = 'open err'

	self.send_response(200)
	self.send_header('Content-type', '%s; charset=utf-8' % contentType)
	self.send_header('Content-Length', str(len(text)))
	self.end_headers()
	self.wfile.write(text.encode('utf-8'))

def returnJson(self, res):
	self.send_response(200)
	self.send_header('Content-type', 'application/json')
	self.end_headers()
	text = json.dumps(res)
	self.wfile.write(text.encode('utf-8'))

class MyHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		pr = urllib.parse.urlparse(self.path)
		if pr.query:
			qs = urllib.parse.parse_qs(pr.query)
			key = qs.get('get', [''])[0]
			if key:
				js = loadJson(DATA_JSON)
				returnJson(self, js.get(key, {}))
			else:
				returnJson(self, {})
		else:
			path = pathlib.Path(pr.path)
			returnFile(self, path)

	def do_POST(self):
		pr = urllib.parse.urlparse(self.path)
		print('PR', pr)
		content_len=int(self.headers.get('content-length'))
		text = self.rfile.read(content_len)
		print('TEXT', text)
		inJson = json.loads(text.decode('utf-8'))
		op = pr.path[1:]
		js = {}
		if op == 'add':
			js = loadJson(DATA_JSON)
			js.update(inJson)
			saveJson(DATA_JSON, js)
		elif op == 'del':
			js = loadJson(DATA_JSON)
			for k in inJson.keys():
				print('delete', k)
				js.pop(k, None)
			saveJson(DATA_JSON, js)
			print(json.dumps(js))

		self.send_response(200)
		self.send_header('Content-type', 'application/json')
		self.end_headers()
		self.wfile.write(json.dumps(js).encode('utf-8'))

def main():
	server = HTTPServer(('localhost', 8000), MyHandler)
	server.serve_forever()

if __name__ == '__main__':
	main()
