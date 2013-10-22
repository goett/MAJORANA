//---------------------------------------------------------------------------//
//                                                                           //
//                                 MGDO                                      //
//                                                                           //
//    This code is the intellectual property of the Majorana and             //
//    GERDA Collaborations.                                                  //
//                                                                           //
//                        *********************                              //
//                                                                           //
//    Neither the authors of this software system, nor their employing       //
//    institutes, nor the agencies providing financial support for this      //
//    work  make  any representation or  warranty, express or implied,       //
//    regarding this software system or assume any liability for its use.    //
//    By copying, distributing or modifying the Program (or any work based   //
//    on on the Program) you indicate your acceptance of this statement,     //
//    and all its terms.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//

#ifndef _MGWFLIBRARIANDEFAULT_HH
#define _MGWFLIBRARIANDEFAULT_HH

#include "MGVWFLibrarian.hh"
#include "MGWaveform.hh"
#include "MGWaveformLibrary.hh"
#include <iostream>
#include <vector>

// Class Description:
//
// MGWFLibrarianSumPulses sums all the pulses in a MGWaveformLibrary
//
// 21 Oct 2013
// Johnny Goett (goett@lanl.gov)
// 



class MGWFLibrarianSumPulses : public MGVWFLibrarian
{

  public:

    MGWFLibrarianSumPulses(const std::string& = "MGWFLibrarianSumPulses");
    virtual ~MGWFLibrarianSumPulses(){}
    
    virtual bool IsInPlace() { return true; }
    
    // Output of requested operation is the input library 
    virtual void TransformInPlace(MGWaveformLibrary& anInput);    

  
    /**************** "Transform" operations: **************/
    

    /**************** Set input variables ***************************/    

      //! Set a pointer to an input waveform for an operation    
    virtual inline void SetInputWaveform( MGWaveform* wf ){ fTmpWaveform = wf; }

       
  protected:
    
    MGWaveform* 		fTmpWaveform;  
    
    virtual void SumPulses( MGWaveformLibrary& lib );

        
}; 



#endif

























