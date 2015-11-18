import requests
import urllib

url='http://ctfq.sweetduet.info:10080/~q31/kangacha.php'

"""
./hashpump -s b5ff24ed3b12bcd01169c1920365397d7568adf25bfb21dd6dcae82c7de93bd00b732e3f0cc5f17370982bd09ff97d255c65c12b459bd443a68b738179a44a19 -d 5 -a ',10' -k 21
"""

c = {
#	'ship':'5',
#	'signature':'b5ff24ed3b12bcd01169c1920365397d7568adf25bfb21dd6dcae82c7de93bd00b732e3f0cc5f17370982bd09ff97d255c65c12b459bd443a68b738179a44a19',
	'ship':urllib.quote('5\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0,10'),
	'signature':'6306e2e1dccbb5bb38b37d8858d06515c430d6a9bba9e6d07194daf8173efbce85c03ea1a926577e90e350ef3d99f3cba0c879dcd16cfe78221c6ee5861fc86a',
}

r = requests.get(url, cookies=c)
print r.text
