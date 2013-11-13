

{
    #include<vector>
    #include<algorithm>

    gSystem->Load("libRooFit");
    using namespace RooFit;

    TChain* S = new TChain("COPPI_Tree");
    //Look at COPPI calibration data from Nov 2013
    S->Add("COPPI_Zr_r1130.root");
    S->Add("COPPI__run1131.root");
    
    TH1D *hc146 = new TH1D("hc146","Channel 146", 2000, 0, 800E3 );
    TH1D *hc147 = new TH1D("hc147","Channel 147", 2000, 0, 800E3 );

   TCanvas* c1 = new TCanvas("c1","",1500,800);
   TCanvas* c2 = new TCanvas("c2","",1500,800);
   c1->Divide(1,2);
   c2->Divide(1,2);
    
   // --- Find Peaks in Spectrum ---
   c1.cd(1);
   S->Draw("SISenergy>>hc146","channel==146");
   c1.cd(2);
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
   
   c1.cd(1);
   TSpectrum *s = new TSpectrum(12);
   Int_t nfound = s->Search(hc146,3,"",0.05);
   vector<Float_t> x146; //TSpectrum guess
   vector<Float_t> xf146; //Fit result
   vector<Float_t> y146; //TSpectrum guess
   vector<Float_t> yf146; //Fit result
   printf("Found %d candidate peaks to fit in channel 146 spectrum:\n",nfound);
   TH1 *hb = s->Background(hc146,20,"same");
   TH1D* hc146bf = hc146->Clone("hc146bf");
   hc146bf->Add(hb,-1);
   if (hb) c1->Update();
   c2->cd(1);
   hc146bf->Draw();;
   Float_t *xpeaks = s->GetPositionX();
   for (i = 0; i < nfound; i++) {
      //printf("%f : %f \n",s->GetPositionX()[i],s->GetPositionY()[i]);
      x146.push_back(s->GetPositionX()[i]);
   }
   sort(x146.begin(),x146.end());
   for(std::vector<Float_t>::iterator it=x146.begin(); it!=x146.end(); ++it)
   {
      tgaus->SetParameter(1,*it);
      TFitResultPtr r = hc146bf->Fit(tgaus,"S+","",*it-0.02*(*it),*it+0.02*(*it));
      xf146.push_back(r->Parameter(1));
      yf146.push_back(r->ParError(1));
   }
   cout << " Ts X \t\t Fit X \t\t err(x) " << endl;
   for(i = 0; i < nfound; i++)
   {
      printf("%f \t %f \t +/- \t %f \n",x146[i],xf146[i],yf146[i]);
   }
   

   c1.cd(2);
   Int_t nfound = s->Search(hc147,3,"",0.05);
   vector<Float_t> x147; //TSpectrum guess
   vector<Float_t> xf147; //Fit result
   vector<Float_t> y147; //TSpectrum guess
   vector<Float_t> yf147; //Fit result
   printf("Found %d candidate peaks to fit in channel 147 spectrum:\n",nfound);
   TH1 *hb2 = s->Background(hc147,20,"same");
   TH1D* hc147bf = hc147->Clone("hc147bf");
   hc147bf->Add(hb2,-1);
   if (hb2) c2->Update();
   c2->cd(2);
   hc147bf->Draw();;
   Float_t *xpeaks = s->GetPositionX();
   for (i = 0; i < nfound; i++) {
      //printf("%f : %f \n",s->GetPositionX()[i],s->GetPositionY()[i]);
      x147.push_back(s->GetPositionX()[i]);
   }
   sort(x147.begin(),x147.end());
   for(std::vector<Float_t>::iterator it=x147.begin(); it!=x147.end(); ++it)
   {
      tgaus->SetParameter(1,*it);
      TFitResultPtr r = hc147bf->Fit(tgaus,"S+","",*it-0.02*(*it),*it+0.02*(*it));
      xf147.push_back(r->Parameter(1));
      yf147.push_back(r->ParError(1));
   }
   cout << " Ts X \t\t Fit X \t\t err(x) " << endl;
   for(i = 0; i < nfound; i++)
   {
      printf("%f \t %f \t +/- \t %f \n",x147[i],xf147[i],yf147[i]);
   }
   

   // --- Extract centroids and estimate backgrounds

   // --- Estimate Intensity

   // --- Fit centroids of peaks with linear function

   // --- Plot residuals

   // --- Build Calibration Curve

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
