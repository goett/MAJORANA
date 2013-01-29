#! /usr/bin/env python
#
""" This will loop over some waveforms,
    calculate the power spectrum, the nosie power spectrum and
    then print them to screen 

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
baselineAverageTime    = 7000;  # ns, average and subtract
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

#c1 = ROOT.TCanvas('c1')               

#start interactive
pylab.ion()
fig=pylab.figure()

for i in range( T.GetEntries() ):
#for i in range(608421):
    T.LoadTree(i)
    T.GetEntry(i)
    event=T.event
    run=T.run
    
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
		fig.clear()
                wf = event.GetWaveform(i_wfm)
                rawwf = wf
                sampling_frequency = wf.GetSamplingFrequency()
                sampling_period    = wf.GetSamplingPeriod()
		if(wf3count==0 and channel==OPPI3ChannelNumber):
	 	     avgwf3.MakeSimilarTo(rawwf)
		if(wf4count==0 and channel==OPPI4ChannelNumber):
	 	     avgwf4.MakeSimilarTo(rawwf)
		if(goodOPPI3Waveform):
	 	     rawwf.MakeSimilarTo(avgwf3)
	 	     avgwf3 += rawwf
                     #Sample baseline from baseline window
		     a3base = numpy.ndarray(int(baselineAverageTime/sampling_period),dtype='float',buffer=rawwf.GetData())
		     a3base = a3base/a3base.max()
		     if(wf3count==0): 
			a3bFFT = pylab.rfft(a3base)
		     else: 
			a3bFFT += pylab.rfft(a3base) 
			print a3bFFT*pylab.conj(a3bFFT)
			raw_input("Press Enter to continue")
 			pylab.subplot(2,1,1)
		     	X3=numpy.arange(0,numpy.size(a3bFFT))
	             	pylab.plot(X3,sampling_period*sampling_period*a3bFFT*pylab.conj(a3bFFT),label='OPPI3')                        
		     	pylab.subplot(2,1,2)
                     	pylab.psd(a3base,Fs=100E6);
		     	fig.canvas.draw()
			#pylab.plot(X3,a3bFFT,label='OPPI3')                        
		     #print "OPPI3 ", eventTime , avgwf3.GetLength(), a3.size , a3
		     wf3count+=1
		if(goodOPPI4Waveform):
	 	     rawwf.MakeSimilarTo(avgwf4)
		     avgwf4 += rawwf
                     #Sample baseline from baseline window
		     a4base = numpy.ndarray(int(baselineAverageTime/sampling_period),dtype='float',buffer=rawwf.GetData())
		     if(wf4count==0): 
			a4bFFT = pylab.rfft(a4base)
		     else:
			a4bFFT += pylab.rfft(a4base) 
		     	pylab.subplot(2,1,1)
		     	X4=numpy.arange(0,numpy.size(a4bFFT))
	             	pylab.plot(X4,sampling_period*sampling_period*a4bFFT*pylab.conj(a4bFFT),label='OPPI4')                        
		     	pylab.subplot(2,1,2)
                     	pylab.psd(a4base,Fs=100E6);
		     	fig.canvas.draw()
			#pylab.plot(X3,a3bFFT,label='OPPI3')                        
			#pylab.plot(X4,a4bFFT,label='OPPI4')                        
		     #print "OPPI4 ", eventTime , avgwf4.GetLength(), a4.size , a4
		     wf4count+=1


a3 = numpy.ndarray(avgwf3.GetLength(),dtype='float',buffer=avgwf3.GetData())
a4 = numpy.ndarray(avgwf4.GetLength(),dtype='float',buffer=avgwf4.GetData())
a3bFFT=a3bFFT/a3bFFT.max()
a4bFFT=a4bFFT/a4bFFT.max()
#raw_input("Press Enter to view both waveforms and their NPS...")
pylab.subplot(3,1,1)
a4=a4/a4.max()
a3=a3/a3.max()
X1=numpy.arange(0,numpy.size(a3))
pylab.plot(X1,a3,label='OPPI3')                        
X2=numpy.arange(0,numpy.size(a4))
pylab.plot(X2,a4,label='OPPI4')
pylab.legend(loc='upper left')
pylab.ylim(0,a4.max()*1.1)
pylab.subplot(3,1,2)
#X2=numpy.arange(0,numpy.size(a4))
a3psd=pylab.psd(a3,Fs=100E6)
a4psd=pylab.psd(a4,Fs=100E6)
#NPS of noise
pylab.subplot(3,1,3)
nps3 = a3bFFT*pylab.conj(a3bFFT)
npsx3=numpy.arange(0,numpy.size(nps3))
nps4 = a4bFFT*pylab.conj(a3bFFT)
npsx4=numpy.arange(0,numpy.size(nps4))
pylab.plot(npsx3,nps3,label='OPPI3')                        
pylab.plot(npsx4,nps4,label='OPPI3')                        
fig.canvas.draw()                        
raw_input("Press Enter to quit...")                        

#newfile.Write()  # write all histograms to disk
newfile.Close()  
