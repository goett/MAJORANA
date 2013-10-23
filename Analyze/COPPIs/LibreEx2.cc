/*
* Johnny Goett (goettj@lanl.gov)
* 9.18.2013
* 
* Description: Produce average waveforms for determination of pulse shape via Librarian class, use
* MGWFLibrarianSumPulses to do the dirty work.
* 
* In: Path to MJOR produced root file
*
* Out: Root file containing a TFile with histograms of average waveforms.
*
* TODO: Tag pile-up events and eliminate from sample.
*/


//ROOT INCLUDES
#include "TFile.h"
#include "TChain.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TStopwatch.h"

//MGDO INCLUDES
#include "MGTEvent.hh"
#include "MGRun.hh"
//#include "MJSIS3302DigitizerData.hh"
//and in particular MGDO Transforms
#include "MGVDigitizerData.hh"
#include "MGTWaveform.hh"
#include "MGWaveformLibrary.hh"
#include "MGWFTrapSlopeFilter.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFTrapezoidalFilter.hh"
#include "MGWFBaselineRemover.hh"
#include "MGWFDerivative.hh"
#include "MGWFLibrarianDefault.hh"

//GAT INCLUDES
#include "GATWaveformTransformer.hh"
#include "GATWFExtremumProcessor.hh"
#include "GATPostedDataTreeWriter.hh"
#include "GATWFTimePointProcessor.hh"
#include "GATWFLinearFitProcessor.hh"

using namespace std;
using namespace CLHEP;

int main(int argc, char** argv)
{
  //Use TApplication to handle plotting with ROOT graphics
  TApplication theApp("App",&argc,argv);
  TStopwatch* watch = new TStopwatch();
  
  // Set up I/O 
  // Give usage info for no input (user must supply an input file)
  // Parse command line for input files and output file
  if(theApp.Argc() < 3) {
    cout << "Usage: " << argv[0] << " <file/s> <output file>" << endl;
    return 1;
  }
  TFile* oF = new TFile(theApp.Argv(theApp.Argc()-1),"RECREATE");
  oF->cd();

  TChain* dataChain = new TChain("MGTree");
  TCanvas* c1 = new TCanvas("c1",theApp.Argv(theApp.Argc()-1),900,800);
  c1->Divide(2,2);

  dataChain->SetDirectory(0);
  for(int i = 1; i<(theApp.Argc()-1); i++)
  {
    dataChain->Add(theApp.Argv(i));
    cout <<"File: " << theApp.Argv(i) << " added to analysis chain." << endl;
  }
   
  //Set Branch Addresses for MGTree
  MGTEvent* event = new MGTEvent();
  dataChain->SetBranchAddress("event",&event);

  //Create placeholders for average waveforms
  MGWaveformLibrary* p1332c2 = new MGWaveformLibrary(2);
  MGWaveformLibrary* p1332c3 = new MGWaveformLibrary(3);
 
  // Loop over events in dataChain 
  // This is where analysis 
  int ch2wfs = 0;
  int ch3wfs = 0;
  int events = dataChain->GetEntries();
  for(int ientry = 0; ientry < events; ientry++)
  {
    dataChain->LoadTree(ientry);
    dataChain->GetEntry(ientry);

    // MGTree contains MGTEvent objects
    int numwaveforms = event->GetNWaveforms();
    //int runNum = run->GetRunNumber(); // This don't work.
    double totalenergy = event->GetETotal(); // Is this summing SISenergy for each waveform in the event?

    // Each event can have more than one waveform. Loop over them.
    for(int i_wfm = 0; i_wfm < numwaveforms; i_wfm++)
    {
      MGVDigitizerData* sisData = event->GetDigitizerData( i_wfm );
      Double_t sisEnergy = sisData->GetEnergy();
      Int_t    channel   = sisData->GetChannel();
      if( i_wfm == 0)
      { 
      	MGTWaveform* rawwf = event->GetWaveform(i_wfm);
        bool pileup = 0; //Assume no pile-up by default. 
        // energy is high just to get 60Co peaks
        // Add waveforms to average for channel 2
        if(sisEnergy>370E3 && sisEnergy<390E3 && channel == 2)
	{
		if(!pileup)
		{
			p1332c2->AddPulseToLibrary(*rawwf);
                        ch2wfs++;
		}
	}
        // Add waveforms to average for channel 3
        if(sisEnergy>365E3 && sisEnergy<370E3 && channel == 3)
	{
		if(!pileup)
		{
			p1332c3->AddPulseToLibrary(*rawwf);
                        ch3wfs++;
		}
	}
      }
      
    }
    
    // You have to do this to get TApplication to update the canvas
    gSystem->ProcessEvents();
    c1->Update();
    // Inform user of progress...
    if(ientry%1000 == 0 ) cout << " Processed " << ientry << " / " << events << '\r'; 
  }  
  cout << " Process Complete. " << endl;

  // Stop timer and print results.
  watch->Stop();
  watch->Print();
  // Save Libraries to file.... well you can't. 
  oF->cd();
  cout << p1332c2->GetNBooks() << " Waveforms added to channel 2 library " << endl;
  cout << p1332c3->GetNBooks() << " Waveforms added to channel 3 library " << endl;

  cout << " Transform waveform libraries " << endl;
  MGWFLibrarianDefault summer("LibrarianTester");
  summer.SetSumPulseToLibrary();
  summer.TransformInPlace(*p1332c2);
  summer.TransformInPlace(*p1332c3);
  cout << p1332c2->GetNBooks() << " Waveforms remaining in channel 2 library " << endl;
  cout << p1332c3->GetNBooks() << " Waveforms remaining in channel 3 library " << endl;
  
   
  oF->Write();
  oF->Close();  

  theApp.Run();
  theApp.Terminate(0);
  return 0;
}
     
