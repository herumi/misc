import subprocess, sys

def main():
	if len(sys.argv) != 3:
		print "run exe file"
		exit(1)
	exe = sys.argv[1]
	name = sys.argv[2]
	f = open(name)
	for line in f:
		line = line.split()
		print line[0], line[1]
		try:
			p = subprocess.check_output([exe, line[1]])
			if p.find("0609") >= 0:
				print "found", line[0]
				break
		except:
			print "err"

if __name__ == '__main__':
    main()



