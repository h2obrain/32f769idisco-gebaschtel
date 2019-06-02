#!/usr/bin/env python3

from sys import argv
from os.path import split,join,isfile

binfile = argv[1]
p,f     = split(binfile)
name    = f.replace(".","_")
hfile   = join(p,name+".h")
if isfile(hfile):
	print("h-file '{}' already exsts!".format(hfile))
	exit(1)


cc = 16
with open(hfile,"w") as f:
	with open(binfile, "rb") as ff:
		f.write("#include <stdint.h>\n")
		f.write("uint8_t {}[] = {{".format(name))
		byte = ff.read(1)
		cc-=1
		c = cc
		while byte:
			if c==cc :
				f.write("\n\t".format(name))
				c=0
			else :
				c+=1
			f.write("0x{:02x},".format(byte[0]))
			byte = ff.read(1)
		f.write("\n};")

