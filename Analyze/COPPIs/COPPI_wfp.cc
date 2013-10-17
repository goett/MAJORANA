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
#include "MGWFTrapSlopeFilter.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFTrapezoidalFilter.hh"
#include "MGWFBaselineRemover.hh"
#include "MGWFDerivative.hh"

//GAT INCLUDES
#include "GATWaveformTransformer.hh"
#include "GATWFExtremumProcessor.hh"
#include "GATPostedDataTreeWriter.hh"
#include "GATWFTimePointProcessor.hh"
#include "GATWFLinearFitProcessor.hh"

using namespace std;
using namespace CLHEP;


//This function is more or less a placeholder and is not ready for use. 
bool PileUpSuspected(MGTWaveform* inform,TCanvas *canvas)
{
	bool pileup = 0;
        //Calculate derivative
        MGTWaveform* dwf = new MGTWaveform();
        dwf->MakeSimilarTo((*inform));
        MGWFDerivative dCalc;
        dCalc.Transform((*inform),(*dwf));
        TH1D* dwfh = dwf->GimmeHist("derivative");
        
        //Look for peaks in derivative using TSpectrum
        TSpectrum *s = new TSpectrum(3,1);
        int nfound = s->Search(dwfh,3,"",0.20);
        if( nfound > 1 )
	{
		cout << "Pileup detected. " << endl;
		pileup = 1;
	}
        return(pileup);
}

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
  //MGRun* run = new MGRun();
  dataChain->SetBranchAddress("event",&event);
  //dataChain->SetBranchAddress("run",&run);

  //Create placeholders for average waveforms
  MGTWaveform* p1332c2 = new MGTWaveform();
  MGTWaveform* p1332c3 = new MGTWaveform();
 
  // Loop over events in dataChain 
  // This is where analysis 
  bool firstPulse2 = 1;
  bool firstPulse3 = 1;
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
    /*
    if( numwaveforms > 1 )
    { 
      //cout << "Waveforms: " << numwaveforms << " Total Energy: " << totalenergy << endl;
    }
    */

    // Each event can have more than one waveform. Loop over them.
    for(int i_wfm = 0; i_wfm < numwaveforms; i_wfm++)
    {
      MGVDigitizerData* sisData = event->GetDigitizerData( i_wfm );
      Double_t sisEnergy = sisData->GetEnergy();
      Int_t    channel   = sisData->GetChannel();
      if( i_wfm == 0)
      { 
      	MGTWaveform* rawwf = event->GetWaveform(i_wfm);
        bool pileup = 0;
        // If this is the first event on channel 2 in the energy window, make placeholder have same base.
        // energy is high just to get 60Co peaks
	if((sisEnergy > 315E3) && firstPulse2 == 1 && channel ==2 )
	{
		p1332c2->MakeSimilarTo((*rawwf));
		firstPulse2 = 0;	
	}
        // Same thing for channel 3
        if((sisEnergy > 315E3) && firstPulse3 == 1 && channel ==3 )
	{
		p1332c3->MakeSimilarTo((*rawwf));
		firstPulse3 = 0;	
	}
        // Add waveforms to average for channel 2
        if(sisEnergy>370E3 && sisEnergy<390E3 && channel == 2)
	{
        	//bool pileup = PileUpSuspected(rawwf);
		if(!pileup)
		{
        		(*p1332c2) += (*rawwf);
        		(*p1332c2) /= 2.0;
                        ch2wfs++;
		}
	}
        // Add waveforms to average for channel 3
        if(sisEnergy>365E3 && sisEnergy<370E3 && channel == 3)
	{
        	//bool pileup = PileUpSuspected(rawwf);
		if(!pileup)
		{
        		(*p1332c3) += (*rawwf);
        		(*p1332c3) /= 2.0;
                        ch3wfs++;
		}
	}
      }
      
    }
    // Every so often, redraw the average.
    if( int(ientry/events) % 100 == 0 )
    {
    	if( p1332c2->GetDuration() && ch2wfs)
    	{
    	    	c1->cd(1);
    		p1332c2->Draw();
    	}
    	if( p1332c3->GetDuration() && ch3wfs)
    	{
    	    	c1->cd(2);
    		p1332c3->Draw();
    	}
        else c1->Update();
    }
    // You have to do this to get TApplication to update the canvas
    gSystem->ProcessEvents();
    c1->Update();
    cout << " Processed " << ientry << " / " << events << '\r'; 
  }  
  cout << " Process Complete. " << endl;
  (*p1332c2)/=ch2wfs;    
  (*p1332c3)/=ch3wfs;    

  // Stop timer and print results.
  watch->Stop();
  watch->Print();
  // Save averages to file. 
  oF->cd();
  TH1D* awfh2 = p1332c2->GimmeHist("ach2");
  TH1D* awfh3 = p1332c3->GimmeHist("ach3");
  awfh2->Write();
  awfh3->Write();
  oF->Write();
  oF->Close();  

  theApp.Run();
  return 0;
}
     
