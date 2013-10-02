/*
* Johnny Goett (goettj@lanl.gov)
* 9.18.2013
* 
* Description: Perform very basic analysis of COPPI data. Based on process_lanl_data
* 
* In: Path to MJOR produced root file
*
* Out: Root file containing a TTree with basic event information.
*
*/


#include "TFile.h"
#include "TChain.h"
#include "GATWaveformTransformer.hh"
#include "GATWFExtremumProcessor.hh"
#include "GATPostedDataTreeWriter.hh"
#include "GATWFTimePointProcessor.hh"
#include "MGWFBaselineRemover.hh"
#include "GATWFLinearFitProcessor.hh"
#include "MGWFTrapSlopeFilter.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFTrapezoidalFilter.hh"

using namespace std;
using namespace CLHEP;

int main(int argc, char** argv)
{
  // Give usage info for no input (user must supply an input file)

  // open input file
  TFile* file = TFile::Open(argv[1]);
  if(file == NULL) return 0;

  // find MGTEvent tree in input file 
  TTree* MGTree = (TTree*) file->Get("MGTree");
  if(MGTree == NULL) {
    cout << "MGTree not found" << endl;
    return 0;
  }

  // Parse command line for input files and output file
  if(argc < 3) {
    cout << "Usage: " << argv[0] << " <file/s> <output file>" << endl;
    return 1;
  }

  TChain* dataChain = new TChain("MGTree");
  dataChain->SetDirectory(0);
  for(int i = 1; i<(argc-1); i++)
  {
    dataChain->Add(argv[i]);
    cout <<"File: " << argv[i] << " added to analysis chain." << endl;
  }
  

  // prepare selector and writer so we can add to them as we go
  TAMSelector selector;
  GATPostedDataTreeWriter DataWriter("COPPI_");

  // add items to copy over from original tree
  DataWriter.AddInputLeafTrivial("fRunNumber", "run");
  DataWriter.AddInputLeafTrivial("fUniqueID", "Event");
  DataWriter.AddInputLeafVector("fDigitizerData.fEnergy", "SISenergy");
  DataWriter.AddInputLeafVector("fDigitizerData.fID", "channel");
  DataWriter.AddInputLeafVector("fDigitizerData.fTimeStamp", "timestamp");



  // wf transform prior to subsequent processing
  MGWFBaselineRemover baselineRemover;
  baselineRemover.SetBaselineSamples(200);
  GATWaveformTransformer baselineRemoverProc(baselineRemover, NULL, "blrwf");
  selector.AddInput(&baselineRemoverProc);
  const char* wfName = "blrwf"; // our "golden" waveform name



  // trap slope filters
  vector<double> tsIntTimes; vector<string> tsitLabels; 
  tsIntTimes.push_back(20.*ns);  tsitLabels.push_back("20ns"); 
  tsIntTimes.push_back(50.*ns);  tsitLabels.push_back("50ns"); 
  tsIntTimes.push_back(100.*ns); tsitLabels.push_back("100ns");
  tsIntTimes.push_back(200.*ns); tsitLabels.push_back("200ns"); 
  tsIntTimes.push_back(500.*ns); tsitLabels.push_back("500ns"); 
  size_t nTSFilters = tsIntTimes.size();
  vector<MGWFTrapSlopeFilter*> tsFilters(nTSFilters);
  vector<GATWaveformTransformer*> tsCurrentProcs(nTSFilters);
  vector<GATWFExtremumProcessor*> tsCurrentMaxProcs(nTSFilters);
  vector<GATWFExtremumProcessor*> tsCurrentMinProcs(nTSFilters);

  for(size_t i=0; i<nTSFilters; i++) {
    tsFilters[i] = new MGWFTrapSlopeFilter;
    tsFilters[i]->SetPeakingTime(10.*ns);
    tsFilters[i]->SetIntegrationTime(tsIntTimes[i]);
    tsFilters[i]->SetEvaluateMode(7);
    tsFilters[i]->OutputInternalParameter("s2");
    string tsWFs = "TSCurrent";
    tsWFs += tsitLabels[i];
    tsCurrentProcs[i] = new GATWaveformTransformer(*(tsFilters[i]), wfName, tsWFs.c_str());
    selector.AddInput(tsCurrentProcs[i]);

    tsCurrentMaxProcs[i] = new GATWFExtremumProcessor(tsWFs.c_str());
    selector.AddInput(tsCurrentMaxProcs[i]);
    DataWriter.AddPostedVector(tsCurrentMaxProcs[i]->GetNameOfPostedVector());

    tsCurrentMinProcs[i] = new GATWFExtremumProcessor(tsWFs.c_str(),false,false);//false false to get the minimum (second false is the minimum)
    selector.AddInput(tsCurrentMinProcs[i]);
    DataWriter.AddPostedVector(tsCurrentMinProcs[i]->GetNameOfPostedVector());


  }

  // rc diff filter for current
  MGWFRCDifferentiation rcDiffFilter;
  rcDiffFilter.SetTimeConstant(50.*ns);
  GATWaveformTransformer rcDiffProc(rcDiffFilter, wfName, "rcdwf");
  selector.AddInput(&rcDiffProc);
  GATWFExtremumProcessor rcDiffMaxProc("rcdwf");
  selector.AddInput(&rcDiffMaxProc);
  DataWriter.AddPostedVector(rcDiffMaxProc.GetNameOfPostedVector());


  selector.AddInput(&DataWriter);
  
  dataChain->Process(&selector);

  return 0;
}
     
