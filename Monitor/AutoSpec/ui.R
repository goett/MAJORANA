library(shiny)

#Define UI
shinyUI(pageWithSidebar(
	#Application Title
	headerPanel("COPPIs - Medium Raw"),
	
	sidebarPanel(
		selectInput("variable","variable:",
			list("Run" = "id", "Channel" = "channel", "Energy" = "integral","baseline" = "baseline", "baseline RMS" = "baseRMS", "10/90 Rise Time"="risetime", "Trap Energy" = "trap"),selected="Energy"),
		#checkboxInput("SUM","Sum channels",TRUE),
		sliderInput("bins","# bins",min=1,max=100000,value=100,step=1),
		downloadButton('downloadData', 'Download Data')
	),

	mainPanel(
		h3(textOutput("caption")),
		plotOutput("specHist")
	)
))
