//---------------------------------------------------------------------------//
//
// INPUT
//
// OUTPUT
//
// Johnny Goett (goatt@lanl.gov) 10.21.2013
//---------------------------------------------------------------------------//

#include "MGWFLibrarianSumPulses.hh"

MGWFLibrarianSumPulses::MGWFLibrarianSumPulses(const std::string& name) : 
 	MGVWFLibrarian(name),
 	fTmpWaveform(NULL)
{
}

void MGWFLibrarianSumPulses::TransformInPlace( MGWaveformLibrary& anInput )
{
      /*!
          This is a very simple transform. Just sum the pulses, there are no options to be parsed.
      */

	SumPulses(anInput);
}


void MGWFLibrarianSumPulses::SumPulses( MGWaveformLibrary& lib )
{

  	while(lib.GetNBooks() > 1 )
	{
		int last = lib.GetNBooks() - 1;
		(*lib[0].GetWaveform()) += (*lib[last].GetWaveform());
		lib.DeleteBookFromLibrary(last+1);	
	} 

}

