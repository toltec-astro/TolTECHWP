""" GALILEVAL

    Program to evaluate data taken by galilcomm.py.

    Log:
    - 2014-5: Used to plot Lens wheel position and HWP position data
    - 2012 ??: Used to plot pupil and lens detent plots

"""

### Compiler commands
import numpy
from pylab import *

### Setup
filename = 'ANoutput.txt'

### Read file
inf = file(filename, 'rt')
lines = inf.readlines()
inf.close()

### Extract data
labels = lines[0].split()
rawdata = numpy.zeros((len(lines)-1,len(labels)))
for i in range(1,len(lines)):
    line = lines[i]
    rawdata[i-1] = [float(s.strip()) for s in line.split()]

times = rawdata[:,0]

### Smooth data (number of points)
nsmth = 5
nold = len(times)
nnew = nold//nsmth
rawnew = numpy.zeros([nnew,len(labels)])
for i in range(nnew):
    for j in range(len(labels)):
        rawnew[i,j]=numpy.median(rawdata[nsmth*i:nsmth*i+nsmth,j])

rawdata = rawnew
times = rawdata[:,0]

### Analog Voltages and motor positions
#   from _MOB _DEB @AN[4] @AN[7] @AN[6] @AN[8] @AN[2] @AN[1]
clf()
# Plot analog values
subplot(2,1,1)
for i in range(3,9):
    plot(times,rawdata[:,i])

xlabel('Time (s)')
ylabel('Analog (V)')
legend(labels[3:9])
title("140520 Forward 90 Steps")
# Plot motor pos and encoder
subplot(2,1,2)
plot(times, rawdata[:,2])
plot(times, rawdata[:,1]*10000)
xlabel('Time (s)')
ylabel('Position (cnt)')

### Analog Voltages scaled 0..0.8 and offset by 1
clf()
for i in range(3,9):
    # get median
    med = numpy.median(rawdata[:,i])
    # plot scale to median = 0.8 with i offset
    plot(times, 0.8*rawdata[:,i]/med+8-i)

xlabel('Time (s)')
ylabel('Analog (V) - smoothed (5), scaled (med=0.8) and offset')
legend(labels[3:9])
title("140520 Forward 90 Steps")

### Analysis of steps vs. signal
plot(rawdata[:,2],rawdata[:,3]) # A1

### Lens wheel (2013)
an1 = rawdata[:,1]
clf()
plot(times,an1)
plot(times,an1,'rd')
xlabel('Time (s)')
ylabel('Analog Input 1')
title(filename)

### Pupil wheel (2013)
an3 = rawdata[:,3]
clf()
plot(times,an3)
plot(times,an3,'rd')
xlabel('Time (s)')
ylabel('Analog Input 3')
title(filename)


