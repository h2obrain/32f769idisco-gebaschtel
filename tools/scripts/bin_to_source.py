#!/usr/bin/env python3

import re
from sys import argv
from os.path import split,join,isfile,basename

def bin_to_array(binfile, hdir=None,hfile=None, cfile=None,overwrite=False):
	p,f     = split(binfile)
	name    = f.replace(".","_")
	if not hdir : hdir = p
	if not hfile :
		hfile_orig = hfile = join(hdir,name+".h")
	else :
		hfile_orig = hfile
		hfile = join(hdir,hfile)
	if not cfile : cfile = name+".c"
	
	if not overwrite :
		if isfile(hfile):
			print("h-file '{}' already exists!".format(hfile))
			exit(1)
		if isfile(cfile):
			print("c-file '{}' already exists!".format(cfile))
			exit(1)
	
	if hfile == cfile :
		print("h/c-file are equal '{}'!".format(hfile))
		exit(1)
	
	#print(hfile,cfile,overwrite)
	
	cname = re.sub(r'(^[^a-zA-Z_])|([^a-zA-Z_\d])',"_",name)
	cc = 16
	with open(cfile,"w") as f:
		with open(binfile, "rb") as ff:
			f.write('#include "{}"\n'
					"__attribute__((__aligned__(8)))\n"
					"const uint8_t {}[] = {{".format(hfile_orig,cname))
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
			f.write("\n"
					"}};\n"
					"const size_t {}_size = sizeof({});\n".format(cname,cname)
				)
	with open(hfile,"w") as f:
		f.write("#include <stdlib.h>\n"
				"#include <stdint.h>\n"
				"extern const uint8_t {}[] __attribute__((__aligned__(8)));\n"
				"extern const size_t {}_size;\n".format(cname,cname))
	
if __name__=="__main__":
	#for binfile in argv[1:]: bin_to_array(binfile)
	
	# TBD: note output file (-o) can only be set for the first file
	import argparse
	
	parser = argparse.ArgumentParser(prog='bin_to_source.py', add_help=False)
	parser.add_argument('-H', '--header_dir', dest='header_dir', action='store', type=str, default=None, help="provides optional output h-file dir")
	parser.add_argument('-h', '--header_file', dest='header_file', action='store', type=str, default=None, help="provides optional output h-file name")
	#parser.add_argument('-C', '--c_dir', dest='c_dir', action='store', type=str, default=None, help="provides optional output C-file dir")
	parser.add_argument('-c', '--c_file', dest='c_file', action='store', type=str, default=None, help="provides optional output c-file name")
	parser.add_argument('-O', '--overwrite', dest='overwrite', action='store_true', default=None, help="overwrite destination file")
	parser.add_argument(dest='file', action='store', type=str, help="provides input file name")
	
	args_ns, remaining = parser.parse_known_intermixed_args()
	bin_to_array(
			args_ns.file,
			hdir=args_ns.header_dir, hfile=args_ns.header_file,
			#cdir=args_ns.c_dir,
			cfile=args_ns.c_file,
			overwrite=args_ns.overwrite)
	while remaining:
		args_ns, remaining = parser.parse_intermixed_args(args=remaining, namespace=args_ns)
		bin_to_array(args_ns.file)
