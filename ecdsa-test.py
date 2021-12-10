import ecdsa
import hashlib

msg = b"hello"
sec = ecdsa.SigningKey.generate(curve=ecdsa.SECP256k1, hashfunc=hashlib.sha256)
pub = sec.verifying_key
sig = sec.sign(msg)
print(f'secHex = "{sec.to_string().hex()}"')
print(f'pubHex = "{pub.to_string().hex()}"')
print(f'sigHex = "{sig.hex()}"')
print(pub.verify(sig, msg))

secHex = "b1aa6282b14e5ffbf6d12f783612f804e6a20d1a9734ffbb6c9923c670ee8da2"
pubHex = "0a09ff142d94bc3f56c5c81b75ea3b06b082c5263fbb5bd88c619fc6393dda3da53e0e930892cdb7799eea8fd45b9fff377d838f4106454289ae8a080b111f8d"
sigHex = "50839a97404c24ec39455b996e4888477fd61bcf0ffb960c7ffa3bef104501919671b8315bb5c1611d422d49cbbe7e80c6b463215bfad1c16ca73172155bf31a"

print('static')
sec = ecdsa.SigningKey.from_string(bytes.fromhex(secHex), curve=ecdsa.SECP256k1, hashfunc=hashlib.sha256)
pub = sec.verifying_key
sig = bytes.fromhex(sigHex)
print(pub.verify(sig, msg))
print(f'secHex = "{sec.to_string().hex()}"')
print(f'pubHex = "{pub.to_string().hex()}"')
print(f'sigHex = "{sig.hex()}"')
