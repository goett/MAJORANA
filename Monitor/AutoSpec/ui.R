library(shiny)

#Define UI
shinyUI(pageWithSidebar(
	#Application Title
	headerPanel("COPPIs - Medium Raw"),
	
	sidebarPanel(
		selectInput("variable","variable:",
			list("Run" = "Run", "Energy" = "Integral", "10/90 Rise Time"="RiseTime"),selected="Energy"),
		#checkboxInput("SUM","Sum channels",TRUE),
		sliderInput("bins","# bins",min=1,max=100000,value=100,step=1)
	),

	mainPanel(
		h3(textOutput("caption")),
		plotOutput("specHist")
	)
))
