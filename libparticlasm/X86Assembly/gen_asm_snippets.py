#!/usr/bin/python

# This is a simple tool to generate a C/C++ header file which is used to embed
# the NASM snippets inside the library.
# Copyright (C) 2012, Leszek Godlewski <github@inequation.org>

import sys, os, re

# this little function:
# * escapes some special characters,
# * replaces '%' signs with literals ('%%'),
# * and writes raw (i.e non-escaped) sequences through, marked by brace
#   characters: '{', '}'
#   - for example: "label: dd {%s}" for use in string formatting,
#   - the braces are stripped in the process, so the above example converts to
#     "label: dd %s"
def escape_for_C(in_str, escape_percent):
    out_str = ""
    raw_string = False
    for c in in_str:
        new_c = c
        if raw_string:
            if c == '}':
                new_c = ""
                raw_string = False
        elif c == '\'':
            new_c = "\\\'"
        elif c == '\"':
            new_c = "\\\""
        elif c == '\n':
            new_c = "\\n"
        elif c == '\r':
            new_c = ""
        elif c == '\t':
            new_c = "\\t"
        elif c == '%' and escape_percent:
            new_c = "%%"
        elif c == '{':
            new_c = ""
            raw_string = True
        out_str = out_str + new_c
    #print(in_str + " -> " + out_str)
    return out_str

asm_path = os.path.abspath(os.path.join("X86Assembly", "AsmSnippets"))

outfile = open(os.path.join("X86Assembly", "AsmSnippets.h"), 'w')
outfile.write("/*\n")
outfile.write("Particlasm assembly snippets header file\n")
outfile.write("AUTOGENERATED - DO NOT MODIFY!!!\n")
outfile.write("If you need to change something, change the corresponding file in the\n")
outfile.write("AsmSnippets directory and re-run gen_asm_snippets.py.\n")
outfile.write("Copyright (C) 2012, Leszek Godlewski <github@inequation.org>\n")
outfile.write("*/\n\n")
outfile.write("#ifndef ASMSNIPPETS_H\n")
outfile.write("#define ASMSNIPPETS_H\n\n")
outfile.write("// In order to include actual source definitions, place a\n")
outfile.write("// \"#define ASMSNIPPETS_DEFINITIONS\" immediately before including this\n")
outfile.write("// header. Otherwise only the extern symbol declarations will be included.\n")
outfile.write("// You don't need to \"#undef ASMSNIPPETS_DEFINITIONS\" afterwards.\n")
outfile.write("#ifdef ASMSNIPPETS_DEFINITIONS\n\n")

dir_list = os.listdir(asm_path)
for fname in dir_list:
    if not fname.endswith(".asm") and not fname.endswith(".inc") :
        continue

    if fname.endswith(".asm"):
        prefix = "Asm_"
    else:
        prefix = "Inc_"

    snippet_name = fname[0:-4]
    outfile.write("extern const char {0}{1}[] =".format(prefix, snippet_name))

    fpath = os.path.join(asm_path, fname);
    print("Processing {0}...".format(os.path.relpath(fpath)))
    line = "foo"
    infile = open(fpath, 'r')
    while line != "":
        line = infile.readline()
        if line != "":
            esc_line = escape_for_C(line, prefix == "Asm_")
            outfile.write("\n\t\"{0}\"".format(esc_line))
    infile.close()
    outfile.write(";\n\n")

outfile.write("#undef ASMSNIPPETS_DEFINITIONS\n\n")
outfile.write("#else // ASMSNIPPETS_DEFINITIONS\n\n")

dir_list = os.listdir(asm_path)
for fname in dir_list:
    if not fname.endswith(".asm") and not fname.endswith(".inc") :
        continue

    if fname.endswith(".asm"):
        prefix = "Asm_"
    else:
        prefix = "Inc_"

    snippet_name = fname[0:-4]
    outfile.write("extern const char {0}{1}[];\n".format(prefix, snippet_name))

outfile.write("\n#endif // ASMSNIPPETS_DEFINITIONS\n\n")
outfile.write("#endif // ASMSNIPPETS_H\n")
outfile.close()