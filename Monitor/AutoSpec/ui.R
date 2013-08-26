library(shiny)

#Define UI
shinyUI(pageWithSidebar(
	#Application Title
	headerPanel("COPPIs - Medium Raw"),
	
	sidebarPanel(
		selectInput("variable","variable:",
			list("Run" = "id", "Channel" = "channel", "Energy" = "integral","baseline" = "baseline", "baseline RMS" = "baseRMS", "10/90 Rise Time"="risetime", "Trap Energy" = "trap"),selected="Energy"),
		checkboxGroupInput("Channels","Active Channels:",c("2"="channel=2","3"="channel=3","5"="channel=5","6"="channel=6"),selected=c('2','3')),
		sliderInput("bins","# bins",min=1,max=100000,value=100,step=1),
		downloadButton('downloadData', 'Download Data')
	),

	mainPanel(
		h3(textOutput("caption")),
		plotOutput("specHist")
	)
))
