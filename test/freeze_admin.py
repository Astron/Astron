# usage
# Astron admin$ python test/freeze_admin.py admin/
# must be run from Astron root

import os
import sys

frozen =  '#include "frozenweb.h"\n'
frozen += 'std::map<std::string,std::string> g_frozenWeb;\n'
frozen += 'void initFrozen(){\n'

files = os.listdir(sys.argv[1])

for filename in files:
    f = open(sys.argv[1]+filename, 'r')
    contents = f.read()
    f.close()
    
    contents = contents.replace("\n", "\\n")
    contents = contents.replace("\"", "\\\"")
    
    frozen += '    g_frozenWeb["/' + sys.argv[1] + filename + '"] = "' + contents + '";\n'
    
    
frozen += "}"

outFile = open("src/http/frozenweb.cpp", "w")
outFile.write(frozen)
outFile.close()