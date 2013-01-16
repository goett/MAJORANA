#!/usr/bin/python
"""
Modified from A. Schubert's viewSISWaveforms.py script.

Usage:  'python drawWaveforms.py [input file(s)]'

Johnny Goett (goett@lanl.gov)

"""
from ROOT import *
TROOT.gApplication.ExecuteFile("$GATDIR/example/MGTEventProcessing/Scripts/rootlogon.C")
TROOT.gApplication.ExecuteFile("$GATDIR/LoadGATClasses.C")
import sys, os, array
from pylab import plt
import numpy


if len(sys.argv) < 2:
  print 'Usage: drawWaveforms.py [ROOT output from MJOR]'
  sys.exit()

# waveform processing parameters:
baselineAverageTime    = 6000    # ns, average and subtract
firstRiseTime          = 0.0
secondRiseTime         = 5500    # ns, time to average for in static window calc
flatTime               = 6000    # ns, time to skip over in static window calc
tau                    = 1/(1.67E-5)  # ns, decay constant (from fit to decay in low gain COPPI data)
energyChannel          = 0
inhibitChannel         = 2
initialRisetimePercent = 0.1
finalRisetimePercent   = 0.9
wfOffsetStartTime      = 1000   # 1 us
wfOffsetEndTime        = 1000   # 1 us
trapFiltFlatTime       = 1000   # 1 us, gap time for trap filter
trapFiltRampTime       = 3000   # 3 us, peaking time, ideal time is probably longer, but we are
                                # limited by the size of the waveform


chain = TChain("MGTree")
chain.SetDirectory(0)

#set up a canvas :
canvas = TCanvas("canvas", "", 580, 700)
canvas.Divide(1,3)

# create a box to show region of baseline averaging:
baseline_box  = TBox()
baseline_box.SetFillStyle(3004)
baseline_box.SetFillColor(TColor.kBlue)
baseline_text = TText()

# create a box to show region of energy calculation:
energy_box  = TBox()
energy_box.SetFillStyle(3004)
energy_box.SetFillColor(TColor.kGreen+3)
energy_text = TText()

# add files to the chain:
for i in range( 1, len(sys.argv) ):
    print 'processing', sys.argv[i]
    chain.Add( sys.argv[i] )

#set up processors:
baseline = MGWFBaselineRemover()

# correct for decay of trace
pz = MGWFPoleZeroCorrection()
pz.SetDecayConstant(tau)

# median filter
medfilt = MGWFMedianFilter()
medfilt.SetSmoothSize(10)

# static window peak height estimate to send to RT calc
sw = MGWFStaticWindow()
sw.SetDelayTime( baselineAverageTime );
sw.SetFirstRampTime( firstRiseTime );
sw.SetSecondRampTime( secondRiseTime );
sw.SetFlatTime( flatTime );

# rise time calculation
riseTimeCalc = MGWFRisetimeCalculation()
riseTimeCalc.SetInitialThresholdPercentage(initialRisetimePercent)
riseTimeCalc.SetFinalThresholdPercentage(finalRisetimePercent)

# trap filter for energy calculation
trapfilt = MGWFTrapezoidalFilter()
trapfilt.SetRampTime(trapFiltRampTime)
trapfilt.SetDecayConstant(tau)
trapfilt.SetFlatTime(trapFiltFlatTime)

# extremum finder
extremum = MGWFExtremumFinder()

# create scratch waveforms:
blrmwf    = MGTWaveform()
trapwf    = MGTWaveform()
rawwf     = MGTWaveform()
pzwf      = MGTWaveform()

#start interactive matplotlib session
plt.ion()

for ientry in xrange( chain.GetEntries() ):
    print ientry
    chain.LoadTree( ientry )
    chain.GetEntry( ientry )

    # tree contains MGTEvent objects
    event = chain.event
    run   = chain.run
    # Two loops because with event building you can have 1+ WF per event, due to the event grouping
    for i_wfm in xrange( event.GetNWaveforms() ):
        sisData   = event.GetDigitizerData( i_wfm )
        channel   = sisData.GetChannel()
        eventTime = sisData.GetTimeStamp()
        runNumber = run.GetRunNumber()
        wf = event.GetWaveform(i_wfm)
        rawwf = wf
        rawWfHist = wf.GimmeHist().Clone()          
        length = wf.GetLength()
                
       
        if (channel >= energyChannel and length>0):
          sampling_frequency = wf.GetSamplingFrequency()
          sampling_period    = wf.GetSamplingPeriod()
          
          # remove baseline:
          baseline.SetStartTime( wfOffsetStartTime )
          baseline.SetBaselineTime( baselineAverageTime )
          baseline.Transform( rawwf, blrmwf )          
          baseline.CalculateBaselineAndRMS( rawwf )
          baselineValue = baseline.GetBaselineMean()
          baselineRMS   = baseline.GetBaselineRMS()
          baseline.Transform( rawwf, trapwf )
          baseline.Transform( rawwf )
          
          #perform trap filter
          trapfilt.Transform( trapwf )
          trapHist = trapwf.GimmeHist().Clone()
          extremum.SetFindMaximum( True )
          extremum.Transform( trapwf )
          trap_energy_nc  = extremum.GetTheExtremumValue() / ( tau*sampling_frequency )
          
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

          # get hists representing waveform objects
	  # trapezoidal filter histogram was extracted above
          pzHist = pzwf.GimmeHist().Clone()
          blHist = blrmwf.GimmeHist().Clone()
	  Rwf = numpy.ndarray(blrmwf.GetLength(),dtype='float',buffer=blrmwf.GetData())
	  Pwf = numpy.ndarray(pzwf.GetLength(),dtype='float',buffer=pzwf.GetData())
	  Twf = numpy.ndarray(trapwf.GetLength(),dtype='float',buffer=trapwf.GetData())
          
	  # Draw if the baseline is something reasonable.  
          #if (baselineValue > 6000):    
          if (1):    
            print '***************************************************'
            print ''
            print '            Energy:  %.2f ADC units' % trap_energy_nc   
            print ''   
            print '    Baseline average     :  %.3f '   % baselineValue
            print '    Baseline RMS         :  %.3f '   % baselineRMS
            print '    Rise Time            :  %.2f ns' % riseTime
            print '    Start of pulse       :  %.2f ns' % startRiseTime
            print ''
            print '***************************************************'
          
            # ----------------------- RAW Waveform ----------------------- #
            canvas.cd(1)
            gPad.SetLeftMargin(0.1)
            gPad.SetRightMargin(0.02)
            titleString1 = "Run:  %i,  Entry: %i, Wfm:  %i, RiseTime:  %.3f #mus, Baseline:  %.2f ADC, " % (runNumber,ientry,i_wfm,riseTime/1000., baselineValue)
            rawWfHist.SetTitle(titleString1)
            rawWfHist.Draw()
            rawWfHist.GetXaxis().SetTitle('Time [ns]')
            rawWfHist.GetYaxis().SetTitle('Voltage [a.u.]')
            rawWfHist.GetYaxis().CenterTitle()
            rawWfHist.GetXaxis().CenterTitle()
            baseline_box.DrawBox(wfOffsetStartTime, rawWfHist.GetMinimum(), baselineAverageTime, rawWfHist.GetMaximum())
                        
            # ----------------------- Pole Zero Corrected Waveform  ----------------------- #
            # draw pz corrected wf with RT start/end and energy averaging window
            canvas.cd(2)
            gPad.SetLeftMargin(0.1)
            gPad.SetRightMargin(0.02)
            pzHist.SetLineColor(2)
            pzHist.Draw()
            blHist.Draw('same')
                              
            f = TF1('f','[0]*TMath::Exp(-(x-%i)/[1])' % 12000,12000,18000)
            f.SetParameter(1,tau)
            f.SetParameter(0,blHist.GetMaximum())
            f.SetParName(0, 'V_{t0}')
            f.SetParName(1, '#tau')
            f.SetLineColor(3)
            f.SetLineWidth(3)
                      
            blHist.Fit( f,'R Q C N' )
                      
            taufit = f.GetParameter(1) / 1000.  # put in microseconds
            titleString2 = "TAU FROM FIT:  V_{t0}*Exp(-(t-t_{0})/#tau):  #tau = %.2f #mus | TAU FROM SCRIPT:  #tau = %.2f #mus" % (taufit,tau/1000.)
            pzHist.SetTitle(titleString2)
                      
            pzHist.GetYaxis().SetTitle('Voltage [a.u.]')
            pzHist.GetXaxis().SetTitle('Time [ns]')
            pzHist.GetYaxis().CenterTitle()
            pzHist.GetXaxis().CenterTitle()
            rtstart = TLine(startRiseTime,blHist.GetMinimum(),startRiseTime,blHist.GetMaximum())
            rtend = TLine(endRiseTime,blHist.GetMinimum(),endRiseTime,blHist.GetMaximum())
            rtstart.SetLineColor(4)
            rtend.SetLineColor(4)
            rtstart.SetLineWidth(2)
            rtend.SetLineWidth(2)
            rtstart.Draw("same")
            rtend.Draw("same")
                    
            energy_averaging_start = baselineAverageTime + wfOffsetStartTime + flatTime
            energy_averaging_stop = energy_averaging_start + secondRiseTime
            energy_box.DrawBox(energy_averaging_start, blHist.GetMinimum(), energy_averaging_stop, blHist.GetMaximum())
                      
            # ----------------------- Trapezoidal Filter ----------------------- #
            canvas.cd(3)
            gPad.SetLeftMargin(0.1)
            gPad.SetRightMargin(0.02)
            titleString3 = 'Trapezoidal Filter (Gap Time:  %i ns,  Peaking Time:  %i ns)' % (trapFiltFlatTime,trapFiltRampTime)
            trapHist.SetTitle(titleString3)
            trapHist.GetXaxis().SetTitle('Time [ns]')
            trapHist.GetYaxis().SetTitle('Voltage [a.u.]')
            trapHist.GetYaxis().CenterTitle()
            trapHist.GetXaxis().CenterTitle()
            trapHist.Draw()

            canvas.Update()

            value = raw_input('  --> Press return to continue, q to quit, p to print, l to see previous wf: ')            
            if value == 'q':
                exit(1)
            if value == 'p':
                waveformName = os.environ['GATDIR'] + '/example/MGTEventProcessing/Data/' + \
                               'Run%sEvent%sWaveform%sChannel%s.pdf' % (runNumber, ientry, i_wfm, channel)
                canvas.Print(waveformName)

#end interactive matplotlib session
plt.ioff()

exit(1)
