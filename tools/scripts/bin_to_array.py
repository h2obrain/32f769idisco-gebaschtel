#!/usr/bin/env python3

import re
from sys import argv
from os.path import split,join,isfile

def bin_to_array(binfile,hfile=None):
	p,f     = split(binfile)
	name    = f.replace(".","_")
	if not hfile:
 		hfile = join(p,name+".h")	
 		if isfile(hfile):
 			print("h-file '{}' already exists!".format(hfile))
 			exit(1)
	
	cc = 16
	with open(hfile,"w") as f:
		with open(binfile, "rb") as ff:
			f.write("#include <stdint.h>\n"
					"__attribute__((__aligned__(8)))\n"
					"const uint8_t {}[] = {{".format(re.sub(r'(^[^a-zA-Z_])|([^a-zA-Z_\d])',"_",name)))
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
	
if __name__=="__main__":
	#for binfile in argv[1:]: bin_to_array(binfile)
	
	# TBD: note output file (-o) can only be set for the first file
	import argparse
	
	parser = argparse.ArgumentParser(prog='bin_to_array.py', add_help=False)
	parser.add_argument('-o', '--output_file', dest='output_file', action='store', type=str, default=None, help="provides optional output file names")
	parser.add_argument(dest='file', action='store', type=str, help="provides input file name")
	
	args_ns, remaining = parser.parse_known_intermixed_args()
	bin_to_array(args_ns.file, args_ns.output_file)
	while remaining:
		args_ns, remaining = parser.parse_intermixed_args(args=remaining, namespace=args_ns)
		bin_to_array(args_ns.file)
