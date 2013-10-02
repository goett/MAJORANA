/*
* Johnny Goett (goettj@lanl.gov)
* 9.18.2013
* 
* Description: Produce average waveforms for determination of pulse shape
* 
* In: Path to MJOR produced root file
*
* Out: Root file containing a TFile with histograms of average waveforms.
*
* TODO: Eliminate pile-up events
*/


#include "TFile.h"
#include "TChain.h"
#include "TH1.h"
#include "TApplication.h"
#include "TSystem.h"
#include "MGTEvent.hh"
#include "MGRun.hh"
//#include "MJSIS3302DigitizerData.hh"
#include "MGVDigitizerData.hh"
#include "GATWaveformTransformer.hh"
#include "GATWFExtremumProcessor.hh"
#include "GATPostedDataTreeWriter.hh"
#include "GATWFTimePointProcessor.hh"
#include "MGWFBaselineRemover.hh"
#include "GATWFLinearFitProcessor.hh"
#include "MGWFTrapSlopeFilter.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFTrapezoidalFilter.hh"
#include "MGTWaveform.hh"

using namespace std;
using namespace CLHEP;

int main(int argc, char** argv)
{
  TApplication theApp("App",&argc,argv);

  // Give usage info for no input (user must supply an input file)
  // Parse command line for input files and output file
  if(theApp.Argc() < 3) {
    cout << "Usage: " << argv[0] << " <file/s> <output file>" << endl;
    return 1;
  }
  TFile* oF = new TFile(theApp.Argv(theApp.Argc()-1),"RECREATE");
  oF->cd();

  TChain* dataChain = new TChain("MGTree");

  dataChain->SetDirectory(0);
  for(int i = 1; i<(theApp.Argc()-1); i++)
  {
    dataChain->Add(theApp.Argv(i));
    cout <<"File: " << theApp.Argv(i) << " added to analysis chain." << endl;
  }
   
  //Set Branch Addresses for MGTree
  MGTEvent* event = new MGTEvent();
  //MGRun* run = new MGRun();
  dataChain->SetBranchAddress("event",&event);
  //dataChain->SetBranchAddress("run",&run);

  //Create average waveforms
  MGTWaveform* p1332c2 = new MGTWaveform();
  MGTWaveform* p1332c3 = new MGTWaveform();
  //MGTWaveform p1332c146();
 
  // Loop over events in dataChain 
  bool firstPulse2 = 1;
  bool firstPulse3 = 1;
  for(int ientry = 0; ientry < dataChain->GetEntries(); ientry++)
  {
    dataChain->LoadTree(ientry);
    dataChain->GetEntry(ientry);

    // MGTree contains MGTEvent objects
    int numwaveforms = event->GetNWaveforms();
    //int runNum = run->GetRunNumber();
    double totalenergy = event->GetETotal();
    if( numwaveforms > 1 )
    { 
      //cout << "Waveforms: " << numwaveforms << " Total Energy: " << totalenergy << endl;
    }

    for(int i_wfm = 0; i_wfm < numwaveforms; i_wfm++)
    {
      MGVDigitizerData* sisData = event->GetDigitizerData( i_wfm );
      Double_t sisEnergy = sisData->GetEnergy();
      Int_t    channel   = sisData->GetChannel();
      if( i_wfm == 0)
      { 
      	MGTWaveform* rawwf = event->GetWaveform(i_wfm);
        if((sisEnergy > 315E3) && firstPulse2 == 1 && channel ==2 )
	{
		p1332c2->MakeSimilarTo((*rawwf));
		firstPulse2 = 0;	
                cout << "Channel: " << channel << endl;
	}
        if((sisEnergy > 315E3) && firstPulse3 == 1 && channel ==3 )
	{
		p1332c3->MakeSimilarTo((*rawwf));
		firstPulse3 = 0;	
                cout << "Channel: " << channel << endl;
	}
        if(sisEnergy>370E3 && sisEnergy<390E3 && channel == 2)
	{
        	(*p1332c2) += (*rawwf);
        	(*p1332c2) /= 2.0;
	}
        if(sisEnergy>365E3 && sisEnergy<370E3 && channel == 3)
	{
        	(*p1332c3) += (*rawwf);
        	(*p1332c3) /= 2.0;
	}
      }
      
    }
  }  

  oF->cd();
  TH1D* awfh2 = p1332c2->GimmeHist("ach2");
  TH1D* awfh3 = p1332c3->GimmeHist("ach3");
  awfh2->Write();
  awfh3->Write();
  oF->Write();
  oF->Close();  

  //theApp.Run();
  //gSystem->ProcessEvents();
  return 0;
}
     
