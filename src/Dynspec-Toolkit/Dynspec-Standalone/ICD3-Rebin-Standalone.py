#!/usr/bin/python
# -*- coding: utf-8 -*-


#############################
# Quick look tool for ICD3 data visualization
# 25/01/2013
# © Nicolas Vilchez for ASTRON
# Nicolas.vilchez@cnrs-orleans.fr
#############################




#############################
# IMPORT COMMANDS
#############################

import sys, os, glob, commands, time
from pylab import *

import getopt
import matplotlib.pyplot as plt
from matplotlib.figure import Figure

import numpy as np
import  h5py
from h5py import *
from dal import *




#############################
# check the Launch
#############################

def main(initparameterlist):

	try:
	      opts, args = getopt.getopt(sys.argv[1:], "hiootttffcR:", ["help", "id=", "obsDIR=", "outputDIR=", "tmin=", "tmax=", "tscale=", "fmin=", "fmax=", "chanPerSubband=", "RAM="])
	      
      
	except getopt.GetoptError as err:
	      print "Usage: ./ICD3-Rebin-Standalone.py --id=Dxxxxxxxxxxxxxxxx.0Zxx...xxx  --obsDIR= --outputDIR=  --tmin= --tmax=  --tscale= --fmin= --fmax= --chanPerSubband= --RAM=(x in Gb)]"
	      print ""
	      sys.exit(2)
	      
	
	for par1, par2 in opts:
		
		if par1 in ("--help"):
			print ""
        		print "Usage: ./ICD3-Rebin-Standalone.py --id=Dxxxxxxxxxxxxxxxx.0Zxx...xxx  --obsDIR= --outputDIR=  --tmin= --tmax=  --tscale= --fmin= --fmax= --chanPerSubband= --RAM=(x in Gb)]"
			print ""
        		sys.exit(2)
        	
		elif par1 in ("--id"):
			initparameterlist[0]=par2
		elif par1 in ("--obsDIR"):
			initparameterlist[1]=par2
		elif par1 in ("--outputDIR"):
			initparameterlist[2]=par2
		elif par1 in ("--tmin"):
			initparameterlist[3]=par2
		elif par1 in ("--tmax"):
			initparameterlist[4]=par2
		elif par1 in ("--tscale"):
			initparameterlist[5]=par2
		elif par1 in ("--fmin"):
			initparameterlist[6]=par2
		elif par1 in ("--fmax"):
			initparameterlist[7]=par2
		elif par1 in ("--chanPerSubband"):
			initparameterlist[8]=par2			
		elif par1 in ("--RAM"):
			initparameterlist[9]=par2									
		else:
        		print("Option {} unknown".format(par1))
        		sys.exit(2)


        		
        # Check parameters		
 	if initparameterlist[0] == "none" or initparameterlist[1] == "none" or initparameterlist[2] == "none" or initparameterlist[3] == "none" or initparameterlist[4] == "none" or initparameterlist[5] == "none" or initparameterlist[6] == "none" or initparameterlist[7] == "none" or initparameterlist[8] == "none" or initparameterlist[9] == "none" or initparameterlist[10] == "none" or initparameterlist[11] == "none":
		print ""
		print "MISSING Parameters"	
		print "Usage: ./ICD3-Rebin-Standalone.py --id=Dxxxxxxxxxxxxxxxx.0Zxx...xxx  --obsDIR= --outputDIR=  --tmin= --tmax=  --tscale= --fmin= --fmax= --chanPerSubband= --RAM=(x in Gb)]"
        	sys.exit(2)       	
  	        	        			
			
	return initparameterlist 


#############################
# ICD3 List Etablishment
#############################

def findData(obsDir,obsID,outputList):
    
      cmd = """find %s -name *%s"""%(obsDir,obsID)+"*bf.h5  > "+"""%s"""%(outputList)
      print cmd
      print commands.getoutput(cmd)
      
      ICD3Inlist	= open(outputList,'r').readlines()
      NbICD3File	= len(ICD3Inlist)
      krange		= range(NbICD3File)
      
      pathFile		= range(NbICD3File)
      pathFiletemp	= range(NbICD3File)
      File		= range(NbICD3File)
      Filetemp		= range(NbICD3File)
      
      i = 0
      for line in ICD3Inlist:
	  splinter		= "L_S"
	  linesplit		= line.split(splinter)
	  pathFiletemp[i]	= linesplit[0]
	  File[i]		= linesplit[1]
	  Filetemp[i]		= linesplit[1]
	  i = i+1
      


      File.sort()
      for i in krange:
	  for j in krange:
	      if File[i] == Filetemp[j]:
		  pathFile[i] = pathFiletemp[j]		  
      
      listTxt	= open(outputList,'w')
      for i in krange:
	  cmd ="""%sL_S%s"""%(pathFile[i],File[i])
	  listTxt.write(cmd)
      listTxt.close()
      
      return outputList

      
#############################
# Matrix for Quicklook
#############################

def matrixGeneration(outputList,outputListDir,obsID,tmin,tmax,tscale,fmin,fmax,chanPerSubband,RAM): 

      ICD3Inlist	= open(outputList,'r').readlines()
      NbICD3File	= len(ICD3Inlist)
      krange		= range(NbICD3File)

      
      # Test if list is empty
      if not ICD3Inlist:
	  print "generated list of ICD3 files is empty:"
	  print """%s observation must not be in %s"""%(obsID,obsDir)
	  print ""
	  exit()      
      
      i = 0
      for line in ICD3Inlist:
	  linesplit	= line.split()
	  h5File 		= File(linesplit[0], File.READWRITE)
	  telescope		= AttributeString(h5File, "TELESCOPE")
	  telescope.value	= "LOFAR" 	  
	  if i == 0:
		Path0 = linesplit[0]
	  i=i+1

	  
      h5File 	= File(Path0, File.READ)     
      SAP	= Group(h5File, "SUB_ARRAY_POINTING_000")
      BEAM	= Group(SAP,"BEAM_000")
      
      DATASET   = DatasetFloat(BEAM, "STOKES_0")
      
      selectCoordinates	= Group(BEAM,"COORDINATES")
      selectSpectral 	= Group(selectCoordinates,"COORDINATE_1")      


      samplingTimeObj	= AttributeFloat(BEAM, "SAMPLING_TIME")
      nofSampleObj	= AttributeInt(DATASET, "NOF_SAMPLES")
      freqCenterObj	= AttributeFloat(BEAM, "BEAM_FREQUENCY_CENTER")
      bandWidthObj	= AttributeFloat(h5File, "BANDWIDTH")      
      chanPerSubbandObj	= AttributeInt(BEAM, "CHANNELS_PER_SUBBAND")
      
      nbSAP	= 1
      nbBEAM	= 1
      nbSTOKES 	= 4
      
      
      duration		= (samplingTimeObj.value)*(nofSampleObj.value)
      freqMin		= (freqCenterObj.value)-((bandWidthObj.value)/2.0)
      freqMax		= (freqCenterObj.value)+((bandWidthObj.value)/2.0)     
      ChannelPerSub	= chanPerSubbandObj.value     
      
      #warning

      if fmin >freqMax:
	  print """The selected frequency minimum is higher than the maximum frequency of the observation: %s MHz"""%(freqMax)
	  print ""
	  print exit()

      if fmax < freqMin:
	  print """The selected frequency maximum is lower than the minimum frequency of the observation: %s Mhz"""%(freqMin)
	  print ""
	  print exit()

	  
      if tmin < 0:
	  print "tmin must be equal to 0 or positive! You are crazy ^^ => New value: 0"
	  print ""
	  tmin = 0    
      
      if tmax > duration:
	  print """tmax must be lower than total integration time => New value is: %s s"""%(round(duration,0)-1)
	  print ""
	  tmax = round(duration,0)-1
	  
	
      if chanPerSubband!= 1:
	   if chanPerSubband&(chanPerSubband-1) !=0 or chanPerSubband<2:
	      print "Channel per subband must be multiple of 2^n or equal to 1"
	      print ""
	      exit()  
	   
      if chanPerSubband > ChannelPerSub:
	   print """Channel per subband must be lower-equal: New value =>  %s"""%(ChannelPerSub)
	   print ""
	   chanPerSubband = ChannelPerSub   
	
      #List-of-Files Output-dir ObsName tmax fmin fmax NofSAP nofBEAM nofStokes
      
      current_dir = os.getcwd()
      
      cmd = """%s/src/ICD3-ICD6-Rebin/DynspecPart_Standalone %s %s %s %s %s %s %s %s %s %s %s %s %s"""%(current_dir,outputList,outputListDir,obsID,tmin,tmax,tscale,fmin,fmax,chanPerSubband,RAM,nbSAP,nbBEAM,nbSTOKES)
      print cmd
      print commands.getoutput(cmd)
      
      


 
    
      
      
#############################
#EXECUTION
#############################
   
if __name__ == '__main__':

  
  
    initparameterlist=range(13)
    
    initparameterlist[0]	= "none"	# Observation ID
    initparameterlist[1]	= "none"	# Directory where to find ICD3 Data
    initparameterlist[2]	= "none"	# Output for quicklook
    initparameterlist[3]	= "none"
    initparameterlist[4]	= "none" 
    initparameterlist[5]	= "none" 
    initparameterlist[6]	= "none" 
    initparameterlist[7]	= "none" 
    initparameterlist[8]	= "none"     
    initparameterlist[9]	= "none" 


    # Read and check parameters	
    initparameterlist = main(initparameterlist);

    ##Parameters
    obsID		= initparameterlist[0]
    obsDir		= initparameterlist[1]
    outputListDir	= initparameterlist[2]
    tmin		= float(initparameterlist[3])
    tmax		= float(initparameterlist[4])
    tscale		= float(initparameterlist[5])
    fmin		= float(initparameterlist[6]) 
    fmax		= float(initparameterlist[7]) 
    chanPerSubband	= int(initparameterlist[8]) 
    RAM			= float(initparameterlist[9])

    
    #Check if output directory exists!
 
    if os.path.isdir(outputListDir) != True:
	cmd = """mkdir %s"""%(outputListDir)
	print commands.getoutput(cmd)   
    
    ### WARNINGS !!
    if outputListDir[-1] != '/':
	outputListDir = outputListDir+'/'
	
    if obsDir[-1] != '/':
	obsDir = obsDir+'/'
		

    ##################""
    ## Processing starts
	
    ##Internal parameter
    outputList		= outputListDir+"ICD3-Rebin-list.txt"	
	
	
    #find ICD3 files 
    start_step1 = time.clock()
    outputList = findData(obsDir,obsID,outputList)
    elapsedstep1 = (time.clock() - start_step1)
    print """ICD3 files list etablished and take: %s s"""%(elapsedstep1)
    print ""
    
    
    #generate rebin data
    matrixGeneration(outputList,outputListDir,obsID,tmin,tmax,tscale,fmin,fmax,chanPerSubband,RAM)
    print ""
    

      

    
    
    
    
    
    
    
    
    
    
    
    
    
    
