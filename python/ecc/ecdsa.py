from ec import Ec, Fp, Fr, initSecp256k1
import hashlib

def byteToFr(b):
	v = 0
	for x in b:
		v = v * 256 + x
	return Fr(v)

def msgToFr(msg):
	H = hashlib.sha256()
	H.update(msg)
	return byteToFr(H.digest())

def sign(P, sec, msg):
	z = msgToFr(msg)
	k = Fr()
	k.setRand()
	Q = P * k
	r = Fr(Q.x.v)
	s = (r * sec + z) / k
	return (r, s)

def verify(P, sig, pub, msg):
	(r, s) = sig
	z = msgToFr(msg)
	w = Fr(1) / s
	u1 = z * w
	u2 = r * w
	Q = P * u1 + pub * u2
	x = Fr(Q.x.v)
	return r == x

def main():
	P = initSecp256k1()
	msg = b"hello"
	sec = Fr(0x83ecb3984a4f9ff03e84d5f9c0d7f888a81833643047acc58eb6431e01d9bac8)
	pub = P * sec

	pubx = 0x653bd02ba1367e5d4cd695b6f857d1cd90d4d8d42bc155d85377b7d2d0ed2e71
	puby = 0x04e8f5da403ab78decec1f19e2396739ea544e2b14159beb5091b30b418b813a
	print("check pub")
	assert pub.x == pubx, "pubx"
	assert pub.y == puby, "puby"

	sig = sign(P, sec, msg)
	print("check verify1, 2")
	assert verify(P, sig, pub, msg), "verify1"
	assert not verify(P, sig, pub, msg + b'x'), "verify2"

	print("check verify3")
	sigr = 0xa598a8030da6d86c6bc7f2f5144ea549d28211ea58faa70ebf4c1e665c1fe9b5
	sigs = 0xde5d79a2ba44e311d04fdca263639283965780bce9169822be9cc81756e95a24
	sig = (Fr(sigr), Fr(sigs))
	assert verify(P, sig, pub, msg), "verify3"
	
if __name__ == '__main__':
	main()
