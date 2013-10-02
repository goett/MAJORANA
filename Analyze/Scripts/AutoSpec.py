#! /usr/bin/env python
#
""" This will loop over events inside MGTree and write as ASCII data file
    for analysis with third party software 

    DAQ configuration:
    channel 2 is unamplified OPPI3
    channel 3 is unamplified OPPI4
    channel 5 is high-gain OPPI3
    channel 6 is high-gain OPPI4

Johnny Goett (goett@lanl.gov) 
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
newfile.write("Run\t Event\t Time\t Channel\t Integral\t Baseline\t BaselineRMS\t RiseTime\t TrapE\t Amp\n")
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
tau                    = 1./(1.67E-5)  # ns, decay constant (from fit to decay in MALBEK)
wfOffsetStartTime      = 1000   # 1 us
wfOffsetEndTime        = 1000   # 1 us
initialRisetimePercent = 0.1
finalRisetimePercent   = 0.9
trapFiltFlatTime       = 1000   # 1 us, gap time for trap filter
trapFiltRampTime       = 3000   # 3 us, peaking time, ideal time may be longer

# create scratch waveforms
blrmwf    = ROOT.MGTWaveform()
rawwf     = ROOT.MGTWaveform()
pzwf      = ROOT.MGTWaveform()
trapwf       = ROOT.MGTWaveform()

#set up processors:
baseline = ROOT.MGWFBaselineRemover()

# correct for decay of trace
pz = ROOT.MGWFPoleZeroCorrection()
pz.SetDecayConstant(tau)

# median filter
medfilt = ROOT.MGWFMedianFilter()
medfilt.SetSmoothSize(10)

# static window peak height estimate to send to RT calc
sw = ROOT.MGWFStaticWindow()
sw.SetDelayTime( baselineAverageTime );
sw.SetFirstRampTime( firstRiseTime );
sw.SetSecondRampTime( secondRiseTime );
sw.SetFlatTime( flatTime );

# rise time calculation
riseTimeCalc = ROOT.MGWFRisetimeCalculation()
riseTimeCalc.SetInitialThresholdPercentage(initialRisetimePercent)
riseTimeCalc.SetFinalThresholdPercentage(finalRisetimePercent)

# trap filter for energy calculation
trapfilt = ROOT.MGWFTrapezoidalFilter()
trapfilt.SetRampTime(trapFiltRampTime)
trapfilt.SetDecayConstant(tau)
trapfilt.SetFlatTime(trapFiltFlatTime)

# extremum finder
extremum = ROOT.MGWFExtremumFinder()

# begin looping events

T = ROOT.TChain('MGTree')
[T.AddFile(name) for name in filenames]                
T.SetCacheSize(10000000)
T.AddBranchToCache("*")
print "Processing ", T.GetEntries(), " Events "

c1 = ROOT.TCanvas('c1')               

for i in range( T.GetEntries() ):
    #status bar
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

        # general cuts
        goodOPPI3Waveform=False
        goodOPPI4Waveform=False 
        if (channel == OPPI3ChannelNumber):
            if InEnergyWindow(OPPI3_EnergyWindow[0],OPPI3_EnergyWindow[1],counts*OPPI3_EnergyToCounts):
               goodOPPI3Waveform=True
               
        elif (channel ==OPPI4ChannelNumber):
            if InEnergyWindow(OPPI4_EnergyWindow[0],OPPI4_EnergyWindow[1],counts*OPPI4_EnergyToCounts):
               goodOPPI4Waveform=True

        if (1 or goodOPPI4Waveform or goodOPPI3Waveform):  #print event information  
	    #print "Run", runNumber, "Event ", i, "Time", eventTime, "Channel ", channel, "Integral ", counts
	   
            wf = event.GetWaveform(i_wfm)
            rawwf = wf
            sampling_frequency = wf.GetSamplingFrequency()
            sampling_period    = wf.GetSamplingPeriod()
            length = wf.GetLength()
	    
	    # remove baseline:
            baseline.SetStartTime( wfOffsetStartTime )
            baseline.SetBaselineTime( baselineAverageTime )
            baseline.Transform( rawwf, blrmwf )
            baseline.CalculateBaselineAndRMS( rawwf )
            baselineValue = baseline.GetBaselineMean()
            baselineRMS   = baseline.GetBaselineRMS()
            baseline.Transform( rawwf, trapwf )
            baseline.Transform( rawwf )
            
	    # compensate for pole-zero:
            pz.Transform( rawwf, pzwf )
            pzwf.SetSamplingFrequency( sampling_frequency )

            #calc rise time and energy
            sw.SetFirstRampTime( firstRiseTime )
            sw.SetDelayTime( baselineAverageTime + wfOffsetStartTime );
            sw.SetSecondRampTime( secondRiseTime )
            sw.SetFlatTime( flatTime )
            sw.Transform( pzwf )
            peakh = sw.GetPeakHeight()
            riseTimeCalc.SetPulsePeakHeight( peakh )
            startSample = int( 3000*sampling_frequency )
            riseTimeCalc.SetScanFrom( startSample )

            # do some smoothing before we calculate the rise time
            medfilt.Transform( pzwf )
            riseTimeCalc.Transform(pzwf)
            startRiseTime = riseTimeCalc.GetInitialThresholdCrossing()*sampling_period
            endRiseTime   = riseTimeCalc.GetFinalThresholdCrossing()*sampling_period
            riseTime      = riseTimeCalc.GetRiseTime()
		
	    #perform trap filter
            trapfilt.Transform( trapwf )
            trapHist = trapwf.GimmeHist().Clone()
            extremum.SetFindMaximum( True )
            extremum.Transform( trapwf )
            trap_energy_nc  = extremum.GetTheExtremumValue() / ( tau*sampling_frequency )
            
            runString=str(runNumber)
	    eventString=str(i)
    	    timeString=str(eventTime)
 	    chString=str(channel)
  	    countString=str(counts)
  	    baselineValueString=str(baselineValue)
  	    baselineRMSString=str(baselineRMS)
  	    riseTimeString=str(riseTime)
 	    if( "inf" in riseTimeString):
                 riseTimeString=str(0)
  	    trapEString=str(trap_energy_nc)
  	    ampString=str(peakh)
            newfile.write(runString + "\t" + eventString + "\t" + timeString + "\t" + chString + "\t" + countString + "\t" + baselineValueString + "\t" + baselineRMSString + "\t" + riseTimeString + "\t" + trapEString + "\t" + ampString + "\n")
                
                        

newfile.close()  
