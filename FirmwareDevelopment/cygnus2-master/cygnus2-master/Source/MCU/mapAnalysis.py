
processFlag = False
fp = open('Listings/MCUDebug.map')
fout = open('MCUDebug.csv','w')
fout.write('Code,data,RO Data,RW Data,ZI Data,Debug,Module,ROM usage,RAM usage\n')
count = 1

while True:
    line = fp.readline()
    if not line:
        break
    if line[0:15] == "Image component sizes"[0:15]:
        processFlag = True
    if line[0:5] == "========"[0:5]:
        processFlag = False
    if processFlag:
        count = count + 1
        strcount = str(count)
        p = line.rstrip().split()
        if len(p) > 6:
            if ".o" in p[6] or "incl." in p[6]:
                fout.write(p[0] + "," + p[1]  + "," + p[2] + "," + p[3] + "," + p[4] + "," + p[5] + "," + p[6] + "," \
                           + "=A" + strcount + "+C" + strcount + "+D" + strcount  + "," + "=D" + strcount + "+E" + strcount + '\n')

fout.close()                
        
    
