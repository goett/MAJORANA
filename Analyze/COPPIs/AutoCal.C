#include<vector>
#include<algorithm>
#include "TH1.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TGraphErrors.h"
#include "TFitResult.h"
#include "RooGlobalFunc.h"
#include "RooFit.h"

using namespace RooFit ;

struct calData{
    Float_t adc; //TSpectrum guess
    Float_t fadc; //Fitted peak position
    Float_t efadc; //error in fitted peak position    
};

bool CompareByadc(const calData& a, const calData& b){
	return a.adc < b.adc;
}

TGraphErrors* ExtractCalCurve(vector<calData>& FitPositions, vector<Float_t>& Energies)
//void ExtractCalCurve(vector<Float_t>& FitPositions, vector<Float_t>& Energies)
{
    //TF1* calFit = new TF1("calFit","[0]*x + [1]",0,800E3);
    TGraphErrors* calCurve = new TGraphErrors(Energies.size());
    
    //Get highest energy peak and estimate [0]
    std::vector<calData>::iterator pit;
    std::vector<Float_t>::iterator eit;
    
    pit = max_element(FitPositions.begin(),FitPositions.end(),CompareByadc);
    eit = max_element(Energies.begin(),Energies.end());
    
    //Extrapolate coefficient and place first point
    int currPoint = 1;
    calCurve->SetPoint(currPoint,(*pit).fadc,*eit);
    calCurve->SetPointError(currPoint,(*pit).efadc,0.0);
    TFitResultPtr r = calCurve->Fit("pol1","SQ");   
    Float_t a = r->Parameter(1);

    Float_t CurrentPeak = 0.0;
    Float_t CurrentEnergy = 0.0;
    Float_t CurrentEnergyEst = 0.0;
    // Loop through found peaks and locate closest estimated energy
    // Assume fitted peaks are already ordered from lowest to highest
    for(std::vector<calData>::iterator i = --(FitPositions.end()); i!=FitPositions.begin(); --i)
    {
        currPoint++;
        CurrentPeak = (*i).fadc;
        CurrentEnergyEst = CurrentPeak*a;
        Float_t CurrentDelta = 800E3;
        for(std::vector<Float_t>::iterator j = Energies.begin(); j!=Energies.end(); j++)
	{
		if( abs(*j - CurrentEnergyEst) < CurrentDelta)
		{
			CurrentDelta = abs(*j - CurrentEnergyEst);
                        CurrentEnergy = *j;
		}
	} 
	
	calCurve->SetPoint(currPoint,CurrentPeak,CurrentEnergy);
        calCurve->SetPointError(currPoint,(*i).efadc,CurrentDelta);
        r = calCurve->Fit("pol1","SQ");
        a = r->Parameter(1);
    }
    r->Print("V");
    return calCurve; 
}

void Play()
{

    //gSystem->Load("libRooFit");
    //using namespace RooFit;

    TChain* S = new TChain("COPPI_Tree");
    //Look at COPPI calibration data from Nov 2013
    S->Add("COPPI_Zr_r1130.root");
    S->Add("COPPI__run1131.root");
    
    TH1D *hc146 = new TH1D("hc146","Channel 146", 2000, 0, 800E3 );
    TH1D *hc147 = new TH1D("hc147","Channel 147", 2000, 0, 800E3 );

   TCanvas* c1 = new TCanvas("c1","",1500,800);
   TCanvas* c2 = new TCanvas("c2","",1500,800);
   TCanvas* c3 = new TCanvas("c3","",1500,800);
   c1->Divide(1,2);
   c2->Divide(1,2);
   c3->Divide(1,2);
    
   // --- Find Peaks in Spectrum ---
   c1->cd(1);
   S->Draw("SISenergy>>hc146","channel==146");
   c1->cd(2);
   S->Draw("SISenergy>>hc147","channel==147");

      // we calibrate with Th and Zr so let's build TSpectrum and look for those peaks
      // let's focus on the highest intensity lines:
      // 208-Th : 2614.5
      // 208-Th : 2103.5 (SEP) too weak in weld rods
      // 208-Th : 1592.5 (DEP)         ''
      // 208-Th : 860
      // 212-Bi : 727
      // 212-Pb : 238
      // 208-Th : 510

   //Use a gaussian to fit each of the peaks.
   TF1* tgaus = new TF1("tgaus","gaus",0,900E3);
   
   c1->cd(1);
   TSpectrum *s = new TSpectrum(12);
   Int_t nfound = s->Search(hc146,3,"",0.05);
   vector<calData> x146; //TSpectrum guess
   printf("Found %d candidate peaks to fit in channel 146 spectrum:\n",nfound);
   TH1 *hb = s->Background(hc146,20,"same");
   TH1D* hc146bf = (TH1D*)hc146->Clone("hc146bf");
   hc146bf->Add(hb,-1);
   if (hb) c1->Update();
   c2->cd(1);
   hc146bf->Draw();;
   Float_t *xpeaks = s->GetPositionX();
   calData d;
   for (int i = 0; i < nfound; i++) {
      //printf("%f : %f \n",s->GetPositionX()[i],s->GetPositionY()[i]);
      d.adc=s->GetPositionX()[i];
      x146.push_back(d);
   }
   sort(x146.begin(),x146.end(),CompareByadc);
   for(std::vector<calData>::iterator it=x146.begin(); it!=x146.end(); ++it)
   {
      tgaus->SetParameter(1,(*it).adc);
      TFitResultPtr r = hc146bf->Fit(tgaus,"SQ+","",(*it).adc-0.02*((*it).adc),(*it).adc+0.02*((*it).adc));
      (*it).fadc=r->Parameter(1);
      (*it).efadc=r->ParError(1);
   }
   cout << " Ts X \t\t Fit X \t\t\t err(x) " << endl;
   for(int i = 0; i < nfound; i++)
   {
      printf("%f \t %f \t +/- \t %f \n",x146[i].adc,x146[i].fadc,x146[i].efadc);
   }
   
   c1->cd(2);
   nfound = s->Search(hc147,3,"",0.05);
   vector<calData> x147; //TSpectrum guess
   printf("Found %d candidate peaks to fit in channel 147 spectrum:\n",nfound);
   TH1 *hb147 = s->Background(hc147,20,"same");
   TH1D* hc147bf = (TH1D*)hc147->Clone("hc147bf");
   hc147bf->Add(hb147,-1);
   if (hb147) c1->Update();
   c2->cd(2);
   hc147bf->Draw();;
   xpeaks = s->GetPositionX();
   for (int i = 0; i < nfound; i++) {
      //printf("%f : %f \n",s->GetPositionX()[i],s->GetPositionY()[i]);
      d.adc=s->GetPositionX()[i];
      x147.push_back(d);
   }
   sort(x147.begin(),x147.end(),CompareByadc);
   for(std::vector<calData>::iterator it=x147.begin(); it!=x147.end(); ++it)
   {
      tgaus->SetParameter(1,(*it).adc);
      TFitResultPtr r = hc147bf->Fit(tgaus,"SQ+","",(*it).adc-0.02*((*it).adc),(*it).adc+0.02*((*it).adc));
      (*it).fadc=r->Parameter(1);
      (*it).efadc=r->ParError(1);
   }
   cout << " Ts X \t\t Fit X \t\t\t err(x) " << endl;
   for(int i = 0; i < nfound; i++)
   {
      printf("%f \t %f \t +/- \t %f \n",x147[i].adc,x147[i].fadc,x147[i].efadc);
   }

   // --- Estimate Intensities

   // --- Fit centroids of peaks with linear function

   // --- Build Calibration Curve

   Float_t thE[] = {2614,2614-511,2614-1022,1620,1512,1282,1093,1078,982,952,927,893,860,821,785,763,748,727,583,510,300,277,238}; 
   vector<Float_t> cE(thE,thE+sizeof(thE)/sizeof(Float_t));

   c3->cd(1);
   TGraphErrors* cal146 = ExtractCalCurve(x146,cE);
   cal146->Draw("AP");
   c3->cd(2);
   TGraphErrors* cal147 = ExtractCalCurve(x147,cE);
   cal147->Draw("AP");
   
   // --- Plot residuals

   // --- Parameters ---
   //RooRealVar adc("adc","SIS bits",360E3,410E3);
 
   //// --- Parameters ---
   //RooRealVar sigmean("sigmean","B^{#pm} mass",387E3,385E3,389E3);
   //RooRealVar sigwidth("sigwidth","B^{#pm} width",2E3,0.25E3,5E3);

   //// --- Build Gaussian PDF ---
   //RooGaussian signal("signal","signal PDF",adc,sigmean,sigwidth) ;
   //// --- Build Exponential PDF ---
   //RooGaussian exp("exp","signal exp",);

   //

}
