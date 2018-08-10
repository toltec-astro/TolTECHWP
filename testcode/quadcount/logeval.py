""" Program to evaluate gaps in Data files

    Usage:
    python logeval.py Datafile.txt



"""

import sys


### Read Lines
lines = [l for l in file(sys.argv[1]) if len (l) > 10]

### Get List of gaps: times and duration
dtlist = [] # List of gap lengths
tlist = [] # List of gap times
# Loop through all lines
for i in range(1, len(lines)-1):
    if 'FiFo' in lines[i]:
        t1 = float(lines[i].split('   ')[1].split(' = ')[1].strip()[:-2])
        t0 = float(lines[i-1].split('   ')[1].split(' = ')[1].strip()[:-2])
        dt = t1-t0
        dtlist.append(dt)
        tlist.append(t1/1000.0)

### Print gaps and get caps statistics
n1 = 0
n3 = 0
n10 = 0
n30 = 0
n100 = 0
for i in range(len(dtlist)):
    print("Time = %7.2fs   dt = %4.1fms" % (tlist[i],dtlist[i]))
    if(dtlist[i]) > 1: n1+=1
    if(dtlist[i]) > 3: n3+=1
    if(dtlist[i]) > 10: n10+=1
    if(dtlist[i]) > 30: n30+=1
    if(dtlist[i]) > 100: n100+=1

print("Total number of gaps: %d" % len(dtlist))
print("Gaps loger than 1ms: %d" % n1)
print("Gaps loger than 3ms: %d" % n3)
print("Gaps loger than 10ms: %d" % n10)
print("Gaps loger than 30ms: %d" % n30)
print("Gaps loger than 100ms: %d" % n100)

