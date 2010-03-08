#
#  VHECRpolarplot.py
#  
#
#  Created by Arthur Corstanje on 1/20/10.
#  Copyright (c) 2010 __MyCompanyName__. All rights reserved.
#

import csv
from mathgl import *
import sys
import os
from math import *
import random
import time

from subprocess import call

timeKey = 'time';           #time = []
phiKey = 'phi';             #phi = []
thetaKey = 'theta';         #theta = []
varianceKey = 'variance';   #variance = []
pointList = []

maximumVarianceAllowed = 40.0
randomizationInDegrees = 1.0 # randomize angular positions with sigma = this number

mglGraphZB = 0
mglGraphPS = 1

numFrames = 1000          # needs to go into the MPEG2.par file!
width=800                 # idem
height=800                # idem; make it square for circle plot

decayTimeInFrames = 25.0
pointSizeInDegreesTheta = 1.5

internalPointsize = 0.8 * pointSizeInDegreesTheta / 90.0 # 0.8 found by trial-and-error to cover the whole 90-degree polar plot

# test plot with color coding

#gX = mglData(100)
#gY = mglData(100)
#gC = mglData(100)
#
#mglGraphPS = 1
#width=800  
#height=800 # make it square for circle plot
#gr = mglGraph(mglGraphPS, width, height)
#
#for i in range(100):
#    gX[i] = i - 50.0
#    gY[i] = (i-50.0)*(i-50.0) / 50.0
#    gC[i] = i
#    
#gr.SetRanges(-50.0, 50.0, 0.0, 50.0, 0.0, 100.0)
#gr.SetCRange(0.0, 100.0)
#gr.SetFontSize(2.0)
#gr.SetMarkSize(0.005) # makes the points smaller
#
#
#gr.Tens(gX, gY, gC, 'Whr o#')
#
#gr.Axis();          
#gr.Grid();
#
#gr.WriteEPS("testColoredPlot.eps","test")

firstTime = 0
lastTime = 0

def processCSVFile(filename): # directly make histogram, not making triggerList
    myKeys = [timeKey, thetaKey, phiKey, varianceKey]
    triggerReader = csv.DictReader(open(fileName), myKeys, delimiter=' ')
    firstRecord = triggerReader.next()
    global firstTime
    global lastTime
    firstTime = int(firstRecord[timeKey])
    if float(firstRecord[varianceKey]) < maximumVarianceAllowed: 
        pointList.append(firstRecord)
    nofPoints = 0
    numberDiscarded = 0
    for record in triggerReader:
        if float(record[varianceKey]) < maximumVarianceAllowed: 
           # phi.append(float(record[phiKey]))
           # theta.append(float(record[thetaKey]))
           # variance.append(float(record[varianceKey]))
            pointList.append(record) # assuming no big files >> 100 MB
            nofPoints += 1
        else:
            numberDiscarded += 1
    print 'Number of points: ' + str(nofPoints)
    print 'Amount of discarded points due to high variance: ' + str(numberDiscarded)
    lastTime = int(pointList[len(pointList) - 1][timeKey])
    print 'First time: ' + str(firstTime)
    print 'Last time: ' + str(lastTime)
if len(sys.argv) > 1:
    fileName = sys.argv[1] 
else:
    fileName = 'VHECRTaskLogfile.dat'   #  that's just handy for rapid testing...
    print 'No filename given; usage: python VHECRpolarplot.py <filename>. Using file ' + fileName + ' by default.'

processCSVFile(fileName)

#gr = mglGraph(mglGraphZB, width, height)
#gr.CirclePts = 1000 # don't know how to improve circle drawing... this is obsolete and not working
#gr.SetFunc("x*cos(y * 2*3.1415926536 / 360)","x*sin(y *(2*3.1415926536) / 360)","");
#gr.SetRanges(0.0, 90, 0.0, 360)
#gr.SetTicks('x', 15.0, 0)
#gr.SetFontSize(2.0)
#gr.SetMarkSize(0.005) # makes the points smaller
#gr.SetCRange(0.0, 1.0) # color code between 0 and 1

datalen = len(pointList)

gX = mglData(datalen)
gY = mglData(datalen)
gC = mglData(datalen)
#for k in range(numFrames):

# X = theta, Y = phi
# theta = 0 is zenit, have to adjust for coincidence log coordinates.
i=0
thisTimeLimit = firstTime
timePerFrame = (lastTime - firstTime) / numFrames
print 'time per frame = ' + str(timePerFrame)

gr = mglGraph(mglGraphZB, width, height) # new object needed?????
gr.SetFunc("x*cos(y * 2*3.1415926536 / 360)","x*sin(y *(2*3.1415926536) / 360)","");
gr.SetRanges(0.0, 90, 0.0, 360)
gr.SetTicks('x', 15.0, 0)
gr.SetFontSize(2.0)
gr.SetMarkSize(internalPointsize) # makes the points smaller
gr.GridPnts = 100
gr.SetCRange(0.0, 1.0) # color code between 0 and 1

for k in range(numFrames):
    print k

    gr.Clf()
    thisTimeLimit += timePerFrame
#    print pointList[i][timeKey]
#    print thisTimeLimit
    
    while int(pointList[i][timeKey]) < thisTimeLimit:
        # add the point to this frame (only x, y; colors come later)
        # do randomization only once !! 
        # make sure that points are added only once, i.e. end with i such that point falls outside the range
        record = pointList[i]
        gX[i] = 90 - float(record[thetaKey]) + random.gauss(0.0, randomizationInDegrees) # set the horizon to 90 degrees
        gY[i] = float(record[phiKey]) + random.gauss(0.0, randomizationInDegrees)
        i += 1
    
    for j in range(i):
        timeOffset = thisTimeLimit - int(pointList[j][timeKey])
        offsetInFrames = float(timeOffset) / float(timePerFrame)
        thisColorCode = exp( - offsetInFrames*offsetInFrames / (decayTimeInFrames * decayTimeInFrames)) 
        #print offsetInFrames
        gC[j] = thisColorCode
        if thisColorCode < 0.03:
            gX[j] = 1000
            gY[j] = 1000
    
    # gX, gY, gC done, now plot it and write to file
    gr.Tens(gX, gY, gC, 'whr o#') # color code light-grey - grey - red (between 0.0 and 1.0), o symbols, solid symbols
    gr.Axis()
    gr.Grid()
    gr.Puts(110.0, 120.0, 0, "Test title!")
    thisRoundedTime = thisTimeLimit / 200e6
    thisRoundedTime = int(thisRoundedTime / 600)
    thisRoundedTime *= 600
    thisTimeString = time.strftime("%a, %d %b %Y %H:%M:%S ", time.gmtime(int(thisTimeLimit / 200e6)))
    gr.Puts(110.0, 60.0, 0, thisTimeString)

#    print 'i = '+ str(i)
    nstring = '%04d' %k
    print 'Writing file ' + nstring
    gr.WritePNG('VHECRframe'+nstring+'.png','', 0)
    gr.Flush()

print 'Calling ImageMagick''s mogrify to convert PNGs into PPM. Takes 1.8 MB per frame...'
call(['mogrify', '-format', 'ppm', '*.png'])
print 'Calling mpeg2vidcodec''s mpeg2encode to make a .mpg MPEG-1 animation out of it'
call(['mpeg2encode', 'MPEG1.par', 'VHECRanimation.mpg'])
print 'Done!'
