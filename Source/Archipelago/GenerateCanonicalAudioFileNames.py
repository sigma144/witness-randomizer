import re
from pathlib import Path

names = dict()

with open("../../App/resource.rc") as file:
    for line in file.readlines():
        line = line.strip()
        if not line.startswith("IDR_WAVE"):
            continue
            
        tokens = re.split(r'\s\s+', line)
        
        names[tokens[0]] = Path(tokens[2][:-1]).stem
        
with open("CanonicalAudioFileNames.h", "w") as outfile:
    outfile.write("""# pragma once

#include "../../App/resource.h"

#include <map>
#include <vector>
#include <string>

inline std::map<int, std::vector<std::string>> canonicalAudioFileNames = {
""")
    for idr, name in names.items():
        outfile.write("\t{ " + idr + ", {")
        if name[-1].isnumeric():
            generic_name = " ".join(name.split(" ")[:-1])
            highest = int(name.split(" ")[-1])
            while highest > 0:
                outfile.write(" \"" + generic_name + " " + str(highest) + ".wav" + "\",")
                highest -= 1
            outfile.write(" \"" + generic_name + ".wav" + "\",")
        else:
            outfile.write(" \"" + name + ".wav" + "\",")
            
        if name.endswith("maj") or name.endswith("Min"):
            generic_name = " ".join(name.split(" ")[:-1])
            outfile.write(" \"" + generic_name + ".wav" + "\",")
        
        outfile.write(" }},\n")

    outfile.write("};\n")
