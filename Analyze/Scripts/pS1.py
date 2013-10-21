#!/usr/bin/python

"""
This script analyzes output from S1 in the COPPI analysis chain.

INPUT: S1 output tree

OUTPUT: plots containing raw spectra 

Johnny Goett 10.16.2013
"""

import ROOT
#ROOT.gROOT.SetBatch(True)
ROOT.PyConfig.IgnoreCommandLineOptions = True
import sys
import os
import glob

# load MGDO so CLHEP is available
ROOT.gApplication.ExecuteFile("$MGDODIR/Root/LoadMGDOClasses.C")
ROOT.gApplication.ExecuteFile("$MGDODIR/Majorana/LoadMGDOMJClasses.C")
from ROOT import CLHEP

# load GAT so classes can be imported from ROOT
ROOT.gApplication.ExecuteFile("$GATDIR/LoadGATClasses.C")

# grab input and output files

if len(sys.argv) < 2:
	print 'Usage: S1b.py <input files>'

# input files 
files = sys.argv[1:-1]
#output file
outFile = ROOT.TFile(sys.argv[-1],"RECREATE")

#Default output tree is 'S1_Tree'
T = ROOT.TChain('S1_Tree')
T.SetDirectory(0)
[T.AddFile(name) for name in files]

print 'Processing ', T.GetEntries(), ' Events in ', files

c1 = ROOT.TCanvas('c1','OPPI 1',800,800)
c2 = ROOT.TCanvas('c2','OPPI 2',800,800)
#Keep python garbage collection from killing the plots
ROOT.SetOwnership(c1,False)
ROOT.SetOwnership(c2,False)

ho1 = ROOT.TH1D('ho1','OPPI 1 SIS energy spectrum', 8000,0.0 ,800E3)
ho2 = ROOT.TH1D('ho2','OPPI 2 SIS energy spectrum', 8000,0.0 ,800E3)

#c1.cd(1)
#T.Draw('SISenergy>>ho1','channel==1')
#c2.cd(1)
#T.Draw('SISenergy>>ho2','channel==2')

#c1.Update()
#c1.Update()

for entry in T:
        #A single event may have more than one signal in the 
        # coincidence window
        for x in range(len(T.channel)):
		if(T.channel[x] ==146): 
			ho1.Fill(T.SISenergy[x])
		if(T.channel[x] ==147):
			ho2.Fill(T.SISenergy[x])

#Save the histograms to file
outFile.cd()
ho1.Write()
ho2.Write()
#now remove directory association so we don't delete them when we close the file
ho1.SetDirectory(0)
ho2.SetDirectory(0)
#now it's safe to close the file
outFile.Close()

#Estimate peak locations
numPeaks = 4
peakFinder = ROOT.TSpectrum(numPeaks)
#Search for peaks in ho1. 
pfound = peakFinder.Search(ho1)
ho1.Draw()
#print table of peak locations
print 'found %d peaks in spectrum' % pfound
print '*************************************'
print ' Position [ADC] \t Count'
for x in xrange(pfound):
	print '%f \t %f' %(peakFinder.GetPositionX()[x],peakFinder.GetPositionY()[x])
print '*************************************'


print ' Objects in memory: '
ROOT.gDirectory.ls()

raw_input("Press Enter to begin interactive session, run quit() when done.")
	


