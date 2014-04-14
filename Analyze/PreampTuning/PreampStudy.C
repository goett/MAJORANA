#include<vector>
#include<algorithm>
#include<utility>
#include "TH1.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TGraphErrors.h"
#include "TFitResult.h"
//#include "RooGlobalFunc.h"
//#include "RooFit.h"

//using namespace RooFit ;

struct calData{
    Float_t adc; //TSpectrum guess
    Float_t fadc; //Fitted peak position
    Float_t efadc; //error in fitted peak position    
};

//we create classes to hold source information
class MJDLine: public std::pair<Float_t,Float_t>{
};

class MJDSource{
    public:
       std::vector< std::pair<Float_t,Float_t> > lines;
       std::vector< std::pair<Float_t,Float_t> >::iterator getIterator();
       void AddLine(Float_t energy, Float_t intensity);
       void AddLines(std::vector<Float_t> energies, std::vector<Float_t> intensities);
       void SetActivity(Float_t avalue);
       void SetName(TString name);
       void SetEIN(TString EIN);
       void Print();

    protected:
       bool CompareByIntensity(const std::pair<Float_t,Float_t>& a, const std::pair<Float_t,Float_t>& b);
       Float_t factivity;
       TString fname;
       TString fEIN;
};

std::vector< std::pair<Float_t,Float_t> >::iterator MJDSource::getIterator(){
	std::vector< std::pair<Float_t,Float_t> >::iterator it = lines.begin();
        return it;
}

void MJDSource::AddLine(Float_t energy, Float_t intensity){
	lines.push_back(std::make_pair(energy,intensity));
        return;	
}

void MJDSource::AddLines(std::vector<Float_t> energies, std::vector<Float_t> intensities){
	if( !(energies.size() == intensities.size()) ){
		std::cout << "The number of lines and intensities do not match." << std::endl;
		return;
	}
	std::vector<Float_t>::iterator eit;	
	std::vector<Float_t>::iterator iit;
	iit = intensities.begin();
	for(eit = energies.begin() ;eit!=energies.end() ;eit++){
		lines.push_back(std::make_pair(*eit,*iit));
 		++iit;
	}
		
	return;	
}

void MJDSource::SetActivity(Float_t avalue){
	factivity = avalue;
	return;
}

void MJDSource::SetName(TString name){
	fname = name;
	return;
}

void MJDSource::SetEIN(TString EIN){
	fEIN = EIN;
	return;
}

void MJDSource::Print(){
	std::vector< std::pair<Float_t,Float_t> >::iterator it;
      		printf("Source %s with EIN: %s \n", fname.Data(), fEIN.Data());
      		printf("%s \t : \t  %s \n","E [keV]","Intensity [\%]");
	for( it = lines.begin(); it!=lines.end(); it++){
      		printf("%f \t : \t  %f \n",(*it).first,(*it).second);
	}
	return;
}

bool MJDSource::CompareByIntensity(const std::pair<Float_t,Float_t>& a, const std::pair<Float_t,Float_t>& b){
	return a.second < b.second;
}

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

    TChain* S = new TChain("mjdTree");
    //Look at COPPI calibration data from Nov 2013
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001923.root");
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001924.root");
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001925.root");
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001926.root");
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001927.root");
    S->Add("/project/projectdirs/majorana/data/mjd/surfprot/data/gatified/P3AKF/mjd_run40001928.root");
    S->Print();
    //S->Add("COPPIsA__run1131.root");
    
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
   S->Draw("energy>>hc146","channel==146");
   c1->cd(2);
   S->Draw("energy>>hc147","channel==147");

      // we calibrate with 133Ba and 60Co so let's build TSpectrum and look for those peaks
      // let's focus on the highest intensity lines:
      // 30.973 0.628
      // 356.0129 0.6205
      // 30.625 0.34
      // 80.9979 0.329
      //
      // 1332.492 0.999826
      // 1173.228 0.9985
      // 8.26-8.33 0.0000136
      // 7.46-7.48 0.000098

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

   // We calibrated with 133Ba and 60Co
   Float_t BaCoHG[] = {383.8485,356.01,302.8508,276.3989,80.9979,79.6142,53.1622}; 
   Float_t BaCo[] = {1332.492,1173.228,1332.492-511,1173-511,356.01,30.973}; 

   c3->cd(1);
   //channel 146 is high gain
   vector<Float_t> cELT(BaCoHG,BaCoHG+sizeof(BaCoHG)/sizeof(Float_t));
   TGraphErrors* cal146 = ExtractCalCurve(x146,cELT);
   cal146->Draw("AP");
   c3->cd(2);
   //channel 147 is normal gain
   vector<Float_t> cE(BaCo,BaCo+sizeof(BaCo)/sizeof(Float_t));
   TGraphErrors* cal147 = ExtractCalCurve(x147,cE);
   cal147->Draw("AP");
   
}
