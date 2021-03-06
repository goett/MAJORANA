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
import numpy
import pylab

# load MGDO so CLHEP is available
ROOT.gApplication.ExecuteFile("$MGDODIR/Root/LoadMGDOClasses.C")
ROOT.gApplication.ExecuteFile("$MGDODIR/Majorana/LoadMGDOMJClasses.C")
from ROOT import CLHEP

# load GAT so classes can be imported from ROOT
ROOT.gApplication.ExecuteFile("$GATDIR/LoadGATClasses.C")
################################################3
inputpath=sys.argv[1]
#filenames=sys.argv[1:-2]
filenames=glob.glob(inputpath)

outputFilename=sys.argv[-1]

print inputpath
print "Inputting ", len(filenames), " files" 

newfile=ROOT.TFile(outputFilename,'RECREATE') # open output right away to enable easy histogram output
###############################################
# create histograms
#none today

# channel assigments
OPPI3ChannelNumber=2
OPPI4ChannelNumber=3

# energy calibration
OPPI3_EnergyToCounts= 1.
OPPI4_EnergyToCounts= 1.  

# Energy Windows
OPPI3_EnergyWindow=[265E3,270E3]
OPPI4_EnergyWindow=[265E3,270E3]

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
avgwf3      = ROOT.MGTWaveform()
avgwf4      = ROOT.MGTWaveform()

wf3count=0
wf4count=0

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
#[T.AddFile(name) for name in filenames]
for name in filenames:
    print "Adding ", name 
    T.AddFile(name)
                
T.SetCacheSize(10000000)
T.AddBranchToCache("*")
print "Processing ", T.GetEntries(), " Events "

c1 = ROOT.TCanvas('c1')               

for i in range( T.GetEntries() ):
#for i in range(608421):
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
            #if InEnergyWindow(OPPI3_EnergyWindow[0],OPPI3_EnergyWindow[1],counts*OPPI3_EnergyToCounts):
            #   goodOPPI3Waveform=True
            goodOPPI3Waveform=True
               
        elif (channel ==OPPI4ChannelNumber):
            #if InEnergyWindow(OPPI4_EnergyWindow[0],OPPI4_EnergyWindow[1],counts*OPPI4_EnergyToCounts):
            #   goodOPPI4Waveform=True
            goodOPPI4Waveform=True

        if (goodOPPI4Waveform or goodOPPI3Waveform):  # draw waveform  
                wf = event.GetWaveform(i_wfm)
                rawwf = wf
                sampling_frequency = wf.GetSamplingFrequency()
                sampling_period    = wf.GetSamplingPeriod()
		if(wf3count==0):
	 	     avgwf3.MakeSimilarTo(rawwf)
		if(wf4count==0):
	 	     avgwf4.MakeSimilarTo(rawwf)
		if(goodOPPI3Waveform):
	 	     rawwf.MakeSimilarTo(avgwf3)
	 	     avgwf3 += rawwf
		     a3 = numpy.ndarray(avgwf3.GetLength(),dtype='float',buffer=avgwf3.GetData())
		     print "OPPI3 ", eventTime , avgwf3.GetLength(), a3.size , a3
		     wf3count+=1
		if(goodOPPI4Waveform):
	 	     rawwf.MakeSimilarTo(avgwf4)
		     avgwf4 += rawwf
		     a4 = numpy.ndarray(avgwf4.GetLength(),dtype='float',buffer=avgwf4.GetData())
		     print "OPPI4 ", eventTime , avgwf4.GetLength(), a4.size , a4
		     wf4count+=1


                
#wfh = avgwf3.GimmeHist()
#wfh.Draw()
#c1.Update()
#raw_input("Press Enter to view average waveform 4...")                        
#wfh = avgwf4.GimmeHist()
#wfh.Draw()
#c1.Update()
raw_input("Press Enter to view both waveforms...")
fig=pylab.figure()
a4=a4/a4.max()
a3=a3/a3.max()
pylab.subplot(2,1,1)
X1=numpy.arange(0,numpy.size(a3))
pylab.plot(X1,a3,label='OPPI3')                        
X2=numpy.arange(0,numpy.size(a4))
pylab.plot(X2,a4,label='OPPI4')
pylab.legend(loc='upper left')
pylab.ylim(0,a4.max()*1.1)
pylab.subplot(2,1,2)
#X2=numpy.arange(0,numpy.size(a4))
a3psd=pylab.psd(a3,Fs=100E6)
a4psd=pylab.psd(a4,Fs=100E6)
#pylab.plot(X2,a4)
pylab.show()                        
raw_input("Press Enter to quit...")                        

#newfile.Write()  # write all histograms to disk
newfile.Close()  
