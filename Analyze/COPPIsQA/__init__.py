from ROOT import *
TROOT.gApplication.ExecuteFile("$GATDIR/example/MGTEventProcessing/Scripts/rootlogon.C")
TROOT.gApplication.ExecuteFile("$GATDIR/LoadGATClasses.C")
import sys, os, array
import numpy
import pylab
import Auto
from RunInfo import *
