#!python3
# -*- coding: utf-8 -*-

# This file is part of the libopencm3 project.
# 
# Copyright (C) 2016 Oliver Meier <h2obrain@gmail.com>
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.
#

from sys import argv
from os.path import isfile, dirname, basename, splitext, join
from math import ceil, floor
import numpy as np
from wand.color import Color
from wand.image import Image
from wand.drawing import Drawing
from wand.compat import nested
from wand.display import display
from posix import fdatasync
from email import charset

print(len(argv),argv)

if len(argv)<3 or len(argv)>4 or (len(argv)==4 and not isfile(argv[3])) :
	print("usage: ./Create_Font.py ./font.ttf fontsize [charsetfile-utf8.txt]")
	print(" - ./font.ttf can also be ./font.pcf or something like Arial or so")
	exit(0)

font = argv[1]
fontsize = None
try :
	fontsize = int(argv[2])
except :
	print("Fontsize is not a number!")
	exit(0)

if len(argv)>=4 :
	charsetfile = argv[3]
else :
	charsetfile = join(dirname(argv[0]),"default_charset.txt")

charset     = open(charsetfile,'r',encoding='utf-8').read().replace('\n','').replace('\r','')
# remove duplicates and sort charset by ord (important for binary search!)  
charset     = ''.join(sorted(set(charset), key=ord))

class fontset_t():
	def __init__(self, name, font,fontsize,antialias, dtype=np.uint32, charset=None):
		fontname       = splitext(basename(font))[0]+"_"+str(fontsize)
		self.fontname       = fontname.replace("-","_")+"_"+name
		print("Creating font", fontname)

		self.font        = font
		self.fontsize    = fontsize
		self.antialias   = antialias
		self.dtype       = dtype
		
		self.chars       = {}
		self.chars_data  = []
		
		self.lineheight  = None
		self.charwidth   = 0
		
		self.dummy_img   = Image(width=1, height=1)
		if charset : self.add_chars(charset)
		
	def add_chars(self, chars):
		for char in sorted(set(chars)):
			print(char, end="")
			self.add_char(char)
		print()
		
	def add_char(self, char):
		if char in self.chars :
			print("Char {} already exists!".format(c))
			return
		with Drawing() as draw:
			draw.font = self.font
			draw.font_size = self.fontsize
			draw.text_antialias = self.antialias
			draw.text_resolution = 72 # point==pixel
			
			metrics = draw.get_font_metrics(self.dummy_img,char)
			if self.lineheight == None :
				self.lineheight = int(ceil(metrics.ascender-metrics.descender))
				self.ascender   = int(ceil(metrics.ascender))
				self.descender  = int(ceil(metrics.descender))
			#if self.charwidth < metrics.character_width :
			#	self.charwidth = int(ceil(metrics.character_width))
			if self.charwidth < metrics.text_width :
				self.charwidth = int(ceil(metrics.text_width))
			
			drop_char = False
			data_idx = -1
			if char==' ' :
				x1=x2=y1=y2 = 0
			else :
				# TODO start using the bbox (have to figure out how it works first!)
				x1 = int(round(metrics.x1))
				x2 = int(round(metrics.x2))
				y1 = int(round(metrics.y1))
				y2 = int(round(metrics.y2))
				w  = int(ceil(metrics.text_width))
				h  = int(ceil(metrics.text_height))
				spatzig = 1
				w+=2*spatzig; x1-=spatzig; x2+=spatzig
				h+=2*spatzig; y1-=spatzig; y2+=spatzig
				# try to align bbox to size..
				while x2-x1 > w :
					w  += 1
					if x2-x1 == w : break
					w  += 2
					x1 += 1
					#if x2-x1 == w : break
					#x2 -= 1
				while x2-x1 < w :
					x1 -= 1
					if x2-x1 == w : break
					x2 += 1
				while y2-y1 > h :
					h  += 1
					if y2-y1 == h : break
					h  += 2
					y1 += 1
					#if y2-y1 == h : break
					#y2 -= 1
				while y2-y1 < h :
					y1 -= 1
					if y2-y1 == h : break
					y2 += 1
				
				#with Image(width=x2-x1, height=y2-y1, background=Color('black')) as img:
				with Image(width=w, height=h, background=Color('black')) as img:
					draw.push()
					draw.fill_color = Color('white')
					draw.text(1,1+int(round(metrics.ascender)), char)
					draw.pop()
					draw(img)
					#display(img)
					d = np.frombuffer(img.make_blob("RGB"),dtype=[('r',np.uint8),('g',np.uint8),('b',np.uint8)])['r'].reshape(img.height,img.width)
					#d = d[y1:,x1:]
					
					if (d==0).all() :
						if char == "_" :
							draw.stroke_color = Color('white')
							draw.stroke_width = 1
							y = int(metrics.ascender+1)
							#print((w,h),(x1+1, y), (x2-1,y))
							#draw.line((x1+1, y), (x2-1,y))
							draw.line((1, y), (w-1,y))
							draw(img)
							display(img)
							d = np.frombuffer(img.make_blob("RGB"),dtype=[('r',np.uint8),('g',np.uint8),('b',np.uint8)])['r'].reshape(img.height,img.width)
							drop_char = (d==0).all()
						else :
							drop_char = True
					
					if not drop_char :
						x1 = y1 = -spatzig
						x2 = w-spatzig
						y2 = h-spatzig
						
						# crop
						for y in range(0,d.shape[0]) :
							if not (d[y,:]==0).all() :
								if y>0 : d = d[y:,:]
								break
							y1 += 1
						for y in range(d.shape[0]-1,0-1,-1) :
							if not (d[y,:]==0).all() :
								d = d[:y+1,:]
								break
							y2 -= 1
						for x in range(0,d.shape[1]) :
							if not (d[:,x]==0).all() :
								if x>0 : d = d[:,x:]
								break
							x1 += 1
						for x in range(d.shape[1]-1,0-1,-1) :
							if not (d[:,x]==0).all() :
								d = d[:,:x+1]
								break
							x2 -= 1
						
						d,x1,x2,y1,y2 = self.fix_data_size_and_bbox(d,x1,x2,y1,y2)

						for i in range(0,len(self.chars_data)) :
							#print("######################################\n",d.shape,"\n",self.chars_data[i].shape)
							#print(d,"\n",self.chars_data[i])
							if (d.shape==self.chars_data[i].shape) and np.all(d==self.chars_data[i]) :
								data_idx = i
								break
							from time import sleep
						
						if data_idx == -1 :
							data_idx = len(self.chars_data)
							self.chars_data.append(d)
			if drop_char :
				print("\b!>{}<!".format(char),end='')
			else :
				self.chars[char] = { 'bbox':(x1,y1,x2,y2), 'data_idx':data_idx }

	#def print_data(self):
	#	for c in self.chars :
	#		print(self.chars_data[k],"==\n\t",k)
	
	def fix_charset(self, charset):
		if not charset :
			charset = self.chars.keys()
		else :
			allc = self.chars.keys()
			ncs = ''
			for c in charset :
				if c in allc : ncs+=c
			charset = ncs
		return charset
	
	def print_charset_chardata(self, charset=None):
		charset = self.fix_charset(charset)
		for c in charset :
			#if not c in self.chars : continue
			self.print_chardata(c)

	def print_chardata(self, c) :
		print(c)
		if not c in self.chars :
			print("\tchar not found!")
			return
		data_idx = self.chars[c]['data_idx']
		if data_idx<0 :
			print("\tchar has no data!")
			return
		d = self.chars_data[data_idx]
		s=np.ndarray(dtype="|S3",shape=d.shape)
		s[d==0]   = '   '
		s[d==255] = "###"
		s[(d>0) & (d<=127)]="---"
		s[(d>127) & (d<255)]="+++"
		for y in range(0,d.shape[0]) :
			print("\t"+s[y,:].tostring().decode("utf8"))

	def create_charset_image(self, savefile=None, charset=None):
		if savefile is None :
			savefile = self.fontname+".png"

		charset = self.fix_charset(charset)
		
		totalwidth = int(ceil(self.charwidth*len(charset)))
		offx = 0
		with Drawing() as draw:
			with Image(width=totalwidth, height=self.lineheight, background=Color('black')) as img:
				for c in charset :
					#if not c in self.chars : continue
					char = self.chars[c]
					data_idx = char["data_idx"]
					if data_idx >= 0 :
						d = self.chars_data[data_idx]
						ddd = np.ndarray(dtype=np.uint8,shape=d.shape+(3,))
						ddd[:,:,0] = d
						ddd[:,:,1] = d
						ddd[:,:,2] = d
						ddd = ddd.tobytes()
						bbox = char["bbox"]
						width=bbox[2]-bbox[0]
						height=bbox[3]-bbox[1]
						#print(c,bbox,(width,height),d.shape,len(ddd))
						with Image(blob=ddd, format='RGB',depth=8, width=width,height=height) as cimg :
							draw.composite(operator='over', left=offx+bbox[0], top=bbox[1], width=width, height=height, image=cimg)
							draw(img)
							#display(img)
					offx += self.charwidth
				img.save(filename=savefile)

	def datasize(self):
		return np.dtype(self.dtype).itemsize*8
# 		if datasize == 64 : return np.uint64
# 		if datasize == 32 : return np.uint32
# 		if datasize == 16 : return np.uint16
# 		if datasize == 8  : return np.uint8
# 		print("invalid data size of '{}'!".format(datasize))
	def fix_data_type(self,u8_data):
		if self.dtype!=np.uint8 :
			e = u8_data.shape[0] % (self.datasize()//8)
			if e :
				u8_data = np.array(u8_data)
				u8_data.resize(u8_data.shape[0] + self.datasize()//8-e)
			u8_data = np.frombuffer(u8_data,dtype=self.dtype)
		return u8_data

	def fix_data_size_and_bbox(self, a8_data, x1,x2,y1,y2):
		if x2-x1 != a8_data.shape[1] :
			print("Invalid x-boundaries, changing x2 from {} to {}".format(x2,x1+a8_data.shape[1]), a8_data.shape)
			x2 = x1 + a8_data.shape[1]
		if y2-y1 != a8_data.shape[0] :
			print("Invalid y-boundaries, changing y2 from {} to {}".format(y2,y1+a8_data.shape[0]), a8_data.shape)
			y2 = y1 + a8_data.shape[0]
		return a8_data, x1,x2,y1,y2

	def pack_data(self, a8_data, verbose=False):
		return None

	def build_c_sources(self):
		chars_table_name  = "chars_"+self.fontname
		data_table_name   = "chars_data_"+self.fontname
		
		chars_table       = "static const char_t "+chars_table_name+"[] = {"
		data_table        = "static const uint"+str(self.datasize())+"_t "+data_table_name+"[] = {"
		
		data_table_offset = 0; data_table_last_comma_idx = -1
		data_fmt = "0x{{:0{}x}},".format(self.datasize()//8*2)
		
		for c in self.chars :
			char = self.chars[c]
			
			# char infos and data
			chars_table+= "\n\t{{ .utf8_value=0x{:08X},".format(ord(c))
			data_table+= "\n\t/* '{:s}' */".format(c)
			
			if char['data_idx']<0:
				# infos for chars without data
				chars_table+= " .bbox={0}, .data=NULL },"
			else :
				# char infos
				chars_table+= " .bbox={{{:2d},{:2d},{:2d},{:2d}}}, .data=&{:s}[{:d}] }},".format(
					*char['bbox'],
					data_table_name,data_table_offset
				)
				# char data
				d = self.pack_data(self.chars_data[char['data_idx']])
				data_table_offset += d.shape[0]
				char_data = ""
				mi = 4
				for v in d :
					mi += 1
					if mi==5 :
						mi=0
						char_data+="\n\t"
					else :
						char_data+=" "
					char_data += data_fmt.format(v)
				data_table += char_data
				data_table_last_comma_idx = len(data_table)-1
		
		chars_table = chars_table[:-1] + "\n};"
		data_table = data_table[:data_table_last_comma_idx] + data_table[data_table_last_comma_idx+1:] + "\n};"
		
		#
		fnu = self.fontname.upper()

		fontsize_definition   = "{:s} {:d}".format(fnu,self.fontsize)
		
		header_filename       = "{:s}.h".format(self.fontname)
		header_file = """
#ifndef _{:s}_
#define _{:s}_

#include <stdint.h>
#include "fonts.h"

extern const font_t font_{:s};

#endif
""".format(
			fnu,fnu,
			self.fontname
		)
		
		c_filename       = "{:s}.c".format(self.fontname)
		c_file = """
#include "{:s}"

{:s}

{:s}

const font_t font_{:s} = {{
	.fontsize       = {:d},
	.lineheight     = {:d},
	.ascender       = {:d},
	.descender      = {:d},
	.charwidth      = {:d},
	.char_count     = {:d},
	.chars          = {:s},
	.chars_data     = {:s},
}};

""".format(
			header_filename,
			data_table,
			chars_table,
			self.fontname, self.fontsize, self.lineheight, self.ascender, self.descender, self.charwidth,
			len(self.chars),chars_table_name, data_table_name,
		)
		
		open(header_filename,'w').write(header_file)
		open(c_filename,'w').write(c_file)

		print("Data table size is: {} bytes".format(data_table_offset * self.datasize()//8))


class mono_fontset_t(fontset_t):
	def __init__(self, font,fontsize, dtype=np.uint32, charset=None):
		super(mono_fontset_t, self).__init__("mono", font,fontsize,False, dtype, charset)
	def pack_data(self, a8_data, verbose=False):
		#a8_data = a8_data.flatten()
		try :
			d = np.packbits(a8_data>127, bitorder='little')
		except :
			d = a8_data.flatten()>127
			e = d.shape[0] % 8
			if e : d.resize(d.shape[0] + 8-e)
			d = np.packbits(np.flip(d.reshape(d.shape[0]//8,8),1))
		return self.fix_data_type(d)

class a4_fontset_t(fontset_t):
	def __init__(self, font,fontsize,antialias=True, dtype=np.uint32, charset=None):
		super(a4_fontset_t, self).__init__("a4", font,fontsize,antialias, dtype, charset)
		if self.datasize()%4 : print("a4_fontset_t: invalid datasize!")
	def fix_data_size_and_bbox(self, a8_data, x1,x2,y1,y2):
		# a4 data needs have even sizes! 
		if (a8_data.shape[0]&1) or (a8_data.shape[1]&1) :
			x2 += a8_data.shape[1]&1
			y2 += a8_data.shape[0]&1
			d = np.zeros(dtype=np.uint8,shape=(a8_data.shape[0]+(a8_data.shape[0]&1), a8_data.shape[1]+(a8_data.shape[1]&1)))
			d[:a8_data.shape[0],:a8_data.shape[1]] = a8_data
			a8_data = d
		return super(a4_fontset_t, self).fix_data_size_and_bbox(a8_data, x1,x2,y1,y2)
	def pack_data(self, a8_data, verbose=False):
		a8_data = a8_data.flatten()
		d = np.ndarray(dtype=np.uint8, shape=(a8_data.shape[0]+1)//2)
		d[:]                     = (a8_data[0::2] >> 4)
		d[:a8_data.shape[0]//2] |= (a8_data[1::2] & 0xf0)
		return self.fix_data_type(d)

class a8_fontset_t(fontset_t):
	def __init__(self, font,fontsize,antialias=True, dtype=np.uint32, charset=None):
		super(a8_fontset_t, self).__init__("a8", font,fontsize,antialias, dtype, charset)
		if self.datasize()%8 : print("a8_fontset_t: invalid datasize!")
	def pack_data(self, a8_data, verbose=False):
		a8_data = a8_data.flatten()
		return self.fix_data_type(a8_data)


mono_fontset = mono_fontset_t(font,fontsize, dtype=np.uint32, charset=charset)
a4_fontset = a4_fontset_t(font,fontsize, charset=charset)
#a8_fontset = a8_fontset_t(font,fontsize, charset=charset)

mono_fontset.create_charset_image(charset=charset)
a4_fontset.create_charset_image(charset=charset)
#a8_fontset.create_charset_image(charset=charset)

mono_fontset.print_charset_chardata(charset=charset)
a4_fontset.print_charset_chardata(charset=charset)
#a8_fontset.print_charset_chardata(charset=charset)

mono_fontset.build_c_sources()
a4_fontset.build_c_sources()
#a8_fontset.build_c_sources()

print("\ndone.")
