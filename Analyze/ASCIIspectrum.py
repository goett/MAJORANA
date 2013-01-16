#! /usr/bin/env python
#
""" This will loop over some waveforms and print them to screen 

    DAQ configuration:
    channel 2 is unamplified OPPI3
    channel 3 is unamplified OPPI4 
"""
########################################################
def InEnergyWindow(lo,hi,value):
    if ((lo <= value) and (value <= hi)):
        return True
    else:
        return False    
########################################################




import ROOT
import sys
import os
import glob

# load MGDO so CLHEP is available
ROOT.gApplication.ExecuteFile("$MGDODIR/Root/LoadMGDOClasses.C")
ROOT.gApplication.ExecuteFile("$MGDODIR/Majorana/LoadMGDOMJClasses.C")
from ROOT import CLHEP

# load GAT so classes can be imported from ROOT
ROOT.gApplication.ExecuteFile("$GATDIR/LoadGATClasses.C")
################################################3
inputpath=sys.argv[1]
filenames=glob.glob(inputpath)

outputFilename=sys.argv[2]

print "Inputting ", len(filenames), " files" 

newfile=open(outputFilename,'wb')
newfile.write("Run \t Event \t Time \t Channel \t Integral \n")
###############################################
# create histograms
#none today

# channel assigments
OPPI3ChannelNumber=2
OPPI4ChannelNumber=3

# energy calibration
OPPI3_EnergyToCounts= 1.00
OPPI4_EnergyToCounts= 1.00  

# Energy Windows
OPPI3_EnergyWindow=[0.00,8200000.00]
OPPI4_EnergyWindow=[0.00,8200000.00]

#useChan2=False


# waveform processing parameters:
baselineAverageTime    = 2500;  # ns, average and subtract
firstRiseTime          = 0;
secondRiseTime         = 5500   # ns, time to average for in static window calc
flatTime               = 11000  # ns, time to skip over in static window calc
tau                    = 59000  # ns, decay constant (from fit to decay in MALBEK)
wfOffsetStartTime      = 1000   # 1 us

# create scratch waveforms
blrmwf    = ROOT.MGTWaveform()
rawwf     = ROOT.MGTWaveform()
pzwf      = ROOT.MGTWaveform()

#set up processors:
baseline = ROOT.MGWFBaselineRemover()

# static window peak height estimate to send to RT calc
sw = ROOT.MGWFStaticWindow()
sw.SetDelayTime( baselineAverageTime );
sw.SetFirstRampTime( firstRiseTime );
sw.SetSecondRampTime( secondRiseTime );
sw.SetFlatTime( flatTime );

# setup complete, begin looping events

T = ROOT.TChain('MGTree')
[T.AddFile(name) for name in filenames]                
T.SetCacheSize(10000000)
T.AddBranchToCache("*")
print "Processing ", T.GetEntries(), " Events "

c1 = ROOT.TCanvas('c1')               

for i in range( T.GetEntries() ):
    T.LoadTree(i)
    T.GetEntry(i)
    event=T.event
    run=T.run
    
    OPPI3_timeStamps=[]
    OPPI4_timeStamps=[]
    
    OPPI3_counts=[]
    OPPI4_counts=[]
    
    OPPI3_energy=[]
    OPPI4_energy=[]
    
    OPPI3_present=False
    OPPI4_present=False
    
    OPPI3_riseTimes50=[]
    OPPI4_riseTimes50=[]
    
    OPPI3_riseTimes90=[]
    OPPI4_riseTimes90=[]
    
    OPPI3_startRiseTimes50=[]
    OPPI4_startRiseTimes50=[]
    
    OPPI3_startRiseTimes90=[]
    OPPI4_startRiseTimes90=[]
    
    OPPI3_endRiseTimes50=[]
    OPPI4_endRiseTimes50=[]
    
    OPPI3_endRiseTimes90=[]
    OPPI4_endRiseTimes90=[]
              
    for i_wfm in xrange( event.GetNWaveforms() ):
        sisData   = event.GetDigitizerData( i_wfm )
        try:
            channel   = sisData.GetChannel()
        except:
            print "Issue with Digitizer Info" 
            print i_wfm, sisData, event.GetNWaveforms(),run,event,i
        eventTime = float(sisData.GetTimeStamp())/float(sisData.GetClockFrequency())
        runNumber = run.GetRunNumber()
        counts=sisData.GetEnergy()
        
        # check for correct channel and energy
        goodOPPI3Waveform=False
        goodOPPI4Waveform=False 
        if (channel == OPPI3ChannelNumber):
            if InEnergyWindow(OPPI3_EnergyWindow[0],OPPI3_EnergyWindow[1],counts*OPPI3_EnergyToCounts):
               goodOPPI3Waveform=True
               
        elif (channel ==OPPI4ChannelNumber):
            if InEnergyWindow(OPPI4_EnergyWindow[0],OPPI4_EnergyWindow[1],counts*OPPI4_EnergyToCounts):
               goodOPPI4Waveform=True

        if (goodOPPI4Waveform or goodOPPI3Waveform):  #print event information  
	    #print "Run", runNumber, "Event ", i, "Time", eventTime, "Channel ", channel, "Integral ", counts
	    runString=str(runNumber)
	    eventString=str(i)
    	    timeString=str(eventTime)
 	    chString=str(channel)
  	    countString=str(counts)
            newfile.write(runString + "\t" + eventString + "\t" + timeString + "\t" + chString + "\t" + countString + "\n")
                
                        

newfile.close()  
