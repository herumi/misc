import requests, md5

uri='/~q9/flag.html'

url = 'http://ctfq.sweetduet.info:10080' + uri
md5a1 = 'c627e19450db746b739f41b64097d449'
nc = '00000001'
cnonce = '9691c249745d94fc'
user = 'q9'

def md5sum(s):
	return md5.new(s).hexdigest()

md5a2 = md5sum("GET:" + uri)

def decodeHeader(s):
	m = {}
	for kv in s.split(','):
		p = kv.find('=')
		k = kv[0:p].strip()
		v = kv[p+1:].strip('"')
		m[k] = v
	return m

r = requests.get(url)
auth = r.headers['WWW-Authenticate']
m = decodeHeader(auth)
realm = m['Digest realm']
nonce = m['nonce']
algo = m['algorithm']
qop = m['qop']

s = "%s:%s:%s:%s:%s:%s" % (md5a1, nonce, nc, cnonce, qop, md5a2)

response = md5sum(s)

param = {
	'Authorization':'Digest username="%s", realm="%s", nonce="%s", uri="%s", algorithm=%s, response="%s", qop=%s, nc=%s, cnonce="%s"' % (user, realm, nonce, uri, algo, response, qop, nc, cnonce),
}

r = requests.get(url, headers=param)
print r.text
