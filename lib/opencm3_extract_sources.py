#!/usr/bin/env python3

# Extract required source by parsing main Makefile

from os.path import join, isfile
from sys import argv, stderr
import re

target_folder = argv[1]

space_m = re.compile(r"\s+")
doublep_m = re.compile(r":")
parser_m = re.compile(r"^\s*(OBJS\s+\+?=\s*(?P<OBJS>.*?)|VPATH\s+\+?=\s*(?P<VPATH>.*?)|include\s+(?P<include>.*?))\s*$",re.MULTILINE)

def find_stuff(folder,makefile, OBJS,VPATH):
    mpth = join(folder,makefile)
    #print("find_stuff({})".format(mpth))
    mf = open(mpth,'r',encoding='utf8').read().replace("\\\n"," ")
    for m in parser_m.finditer(mf):
        if   m.group('OBJS')    : OBJS  += space_m.split(m.group('OBJS'))
        elif m.group('VPATH')   : VPATH += doublep_m.split(m.group('VPATH'))
        elif m.group('include') : find_stuff(folder,m.group('include').strip(), OBJS, VPATH)
    return OBJS,VPATH

OBJS,VPATH = find_stuff(target_folder,"Makefile",[],[''])
#print("OBJS:",OBJS)
#print("VPATH:",VPATH)

error = False
SRCS = []
for o in OBJS :
    c = o[:-1]+"c"
    found = False
    for vp in VPATH :
        src = join(target_folder,vp,c)
        if isfile(src) :
            SRCS.append(src)
            found = True
            break
    if not found :
        error = True
        print("Object '{}' could not be found!".format(o),file=stderr)

print(" ".join(SRCS))
if error : exit(1) # is this possible somehow?

