#!/usr/bin/python

# a simple code generator that translates relevant declarations from libparticlasm.h
# to a NASM include file
# Copyright (C) 2011-2012, Leszek Godlewski <github@inequation.org>

import sys, os, errno

config = {
	# enumerations to extract
	"enums": {
		"ptcDistributionID",
		"ptcModuleID",
		"ptcColourFlags",
		"ptcGravityFlags"
	},
	# structures to extract
	"structs": {
		"ptcSDistr_Constant",
		"ptcSDistr_Uniform",
		"ptcSDistr_BicubicInterp",
		"ptcVDistr_Constant",
		"ptcVDistr_Uniform",
		"ptcVDistr_BicubicInterp",
		"ptcCDistr_Constant",
		"ptcCDistr_Uniform",
		"ptcCDistr_BicubicInterp",
		"ptcModuleHeader",
		"ptcMod_InitialLocation",
		"ptcMod_InitialRotation",
		"ptcMod_InitialSize",
		"ptcMod_InitialVelocity",
		"ptcMod_InitialColour",
		"ptcMod_Velocity",
		"ptcMod_Acceleration",
		"ptcMod_Colour",
		"ptcMod_Size",
		"ptcMod_Gravity",
		"ptcParticle",
		"ptcVertex",
		"ptcEmitterConfig",
		"ptcEmitter"
	},
	# unions to extract
	"unions": {
		"ptcScalarDistr",
		"ptcVectorDistr",
		"ptcColourDistr",
		"ptcModule"
	}
}

# explicit type map
typemap = {
	# C type		primitive   	count
	"ptcID":		["resd",	    1],
	"float":		["resd",	    1],
	"ptcScalar":	["resd",	    1],
	"int16_t":		["resw",	    1],
	"int32_t":		["resd",	    1],
	"uint16_t":		["resw",	    1],
	"uint32_t":		["resd",	    1],
	"ptcVector":	["resd",	    3],
	"ptcColour":	["resd",	    4],
	# platform-dependent (32 vs 64-bit) type macros
	"*":			["reserve(ptr_t)",	1],
	"size_t":       ["reserve(size_t)", 1],
	"ptcModulePtr":	["reserve(ptr_t)",	1]
}

enums = []
structs = []
unions = []

def get_asm_type(ctype, nested):
	if (ctype in typemap):
		return typemap[ctype]
	for ei in enums:
		if (ei["symbol"] == ctype):
			return ["resd", 1]
	for si in structs:
		if (si["symbol"] == ctype):
			if (not nested):
				return ["resb", ctype + "_size"]
			else:
				return get_struct(si)
	for ui in unions:
		if (ui["symbol"] == ctype):
			return get_union(ui)
	return ["", 0]

def get_struct(struct):
	accum = 0
	for m in struct["members"]:
		accum += get_type_size(get_asm_type(m["type"], True)) * m["count"]
	return ["resb", accum]

def get_union(union):
	maximum = 0
	for m in union["members"]:
		i = get_type_size(get_asm_type(m["type"], True)) * m["count"]
		if (i > maximum):
			maximum = i
	return ["resb", maximum]

def get_type_size(t):
	bytes = 1
	if (t[0] == "resw"):
		bytes = 2
	elif (t[0] == "resd"):
		bytes = 4
	return bytes * t[1]

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

# load the C header

infile = open("libparticlasm.h", 'r')
line = "foo"

state = 0

ei = {}
si = {}
ui = {}

print "Scanning libparticlasm.h..."

while (line != ''):
	line = infile.readline()
	l = line.strip()
	if (state == 0):
		if (l.startswith('enum')):
			symbol = l[5 : l.find(" ", 5)]
			if (symbol in config["enums"]):
				state = 1
				ei = {"symbol": symbol, "pairs": []}
				continue
		if (l.startswith('typedef')):
			if (l.startswith('struct', 8)):
				state = 2
				si = {"symbol": "", "members": []}
				continue
			elif (l.startswith('union', 8) and l.find('{') >= 0):
				state = 3
				ui = {"symbol": "", "members": []}
				continue
	elif (state == 1):
		if (l.startswith("}")):
			enums.append(ei)
			state = 0
			continue
		comma = l.find(",")
		if (comma < 0):
			comma = 1024 * 1024 * 1024
		space = l.find(" ")
		if (space < 0):
			space = 1024 * 1024 * 1024
		tab = l.find("\t")
		if (tab < 0):
			tab = 1024 * 1024 * 1024
		comment = l.find("///")
		if (comment < 0):
			comment = 1024 * 1024 * 1024
		delim = min(comma, space, tab, comment, len(l))
		name = l[0 : delim]
		assignment = l.find("=")
		val = 0
		if (assignment >= 0):
			begin = assignment + 1
			while (not l[begin].isdigit()):
				begin = begin + 1
			end = begin + 1
			while (l[end].isdigit()):
				end = end + 1
			val = int(l[begin : end])
		elif (len(ei["pairs"]) > 0):
			val = ei["pairs"][-1]["value"] + 1
		ei["pairs"].append({"name": name, "value": val})
		#print ei["symbol"] + ": " + name + " = " + str(val)
	elif (state == 2):
		if (l.startswith("}")):
			si["symbol"] = l[2 : l.find(";")]
			structs.append(si)
			state = 0
			continue
		tokens = l.split()
		# skip empty lines and comments
		if (len(tokens) == 0 or tokens[0] == "//"):
			continue
		vtype = tokens[0]
		symbol = tokens[1][0 : tokens[1].find(";")]
		if (symbol[0] == "*" or vtype[-1] == "*"):
			vtype = "*"
			if (symbol[0] == "*"):
				symbol = symbol[1 : len(symbol)]
		count = 1
		bracket = symbol.find("[")
		if (bracket >= 0):
			count = int(symbol[bracket + 1 : -1])
			symbol = symbol[0 : bracket]
		si["members"].append({"type": vtype, "symbol": symbol, "count": count})
	elif (state == 3):
		if (l.startswith("}")):
			ui["symbol"] = l[2 : l.find(";")]
			unions.append(ui)
			state = 0
			continue
		tokens = l.split()
		vtype = tokens[0]
		symbol = tokens[1][0 : tokens[1].find(";")]
		if (symbol[0] == "*" or vtype[-1] == "*"):
			vtype = "*"
			if (symbol[0] == "*"):
				symbol = symbol[1 : len(symbol)]
		count = 1
		bracket = symbol.find("[")
		if (bracket >= 0):
			count = int(symbol[bracket + 1 : -1])
			symbol = symbol[0 : bracket]
		ui["members"].append({"type": vtype, "symbol": symbol, "count": count})

infile.close()

print "Done."

#print "\nENUMS:"
#print enums
#print "\nSTRUCTS:"
#print structs
#print "\nUNIONS:"
#print unions

print "Writing libparticlasm.inc..."

outpath = os.path.join("X86Assembly", "AsmSnippets")
mkdir_p(outpath)
outfile = open(os.path.join(outpath, "libparticlasm.inc"), 'w')
outfile.write("; particlasm NASM declarations\n")
outfile.write("; Copyright (C) 2011-2012, Leszek Godlewski <github@inequation.org>\n\n")
outfile.write("; AUTOGENERATED - DO NOT MODIFY!!!\n")
outfile.write("; If you need to change something, change the corresponding libparticlasm.h\n")
outfile.write("; declaration and re-run gen_asm_decls.py.\n")

outfile.write("\n%ifndef LIBPARTICLASM_INC\n")
outfile.write("%define LIBPARTICLASM_INC\n")

outfile.write("\n; enumerations\n")
for ei in enums:
	outfile.write("\n; " + ei["symbol"] + "\n")
	for p in ei["pairs"]:
		outfile.write(p["name"] + "\t\tequ\t" + str(p["value"]) + "d\n")

outfile.write("\n; structures\n")
for si in structs:
	outfile.write("\nstruc " + si["symbol"] + "\n")
	for m in si["members"]:
		t = get_asm_type(m["type"], False)
		outfile.write("\t." + m["symbol"] + ":\t\t" + t[0] + "\t" + str(t[1] * m["count"]) + "\n")
	outfile.write("endstruc\n")

#outfile.write("\n; declarations of entry points\n")
#outfile.write("%ifdef PTC_MODULES\n")
#outfile.write("\t%define visdecl global\n")
#outfile.write("%else\n")
#outfile.write("\t%define visdecl extern\n")
#outfile.write("%endif\n")
#for ei in enums:
#	if (ei["symbol"] != "ptcModuleID"):
#		continue
#	for p in ei["pairs"]:
#		symbol = p["name"][p["name"].find("_") + 1 : len(p["name"])]
#		outfile.write("visdecl mod_" + symbol + "Spawn\n")
#		outfile.write("visdecl mod_" + symbol + "Spawn_size\n")
#		outfile.write("visdecl mod_" + symbol + "Process\n")
#		outfile.write("visdecl mod_" + symbol + "Process_size\n")

outfile.write("\n%endif ; LIBPARTICLASM_INC\n")

outfile.close()

print "Done."
