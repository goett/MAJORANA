def ActiveChannels(dataTree):
	c = []
	for ientry in xrange( dataTree.GetEntries() ):
		dataTree.GetEntry( ientry )
		event = dataTree.event

		for i_wfm in xrange( event.GetNWaveforms()):
			sisData = event.GetDigitizerData( i_wfm )
			if( not(sisData.GetChannel() in c) ):
				c.append(sisData.GetChannel()) 
			
	print c
	return c
