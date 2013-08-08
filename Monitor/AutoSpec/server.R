library(shiny)

load("/export/HomeArray/home/goett/Workspace/COPPI/AutoSpec/AutoSpec.RData")
#str(CFrame)
#attach(CFrame)
#CFrame<-read.table(file='/export/HomeArray/home/goett/Workspace/COPPI/AutoSpec/xxx',header=TRUE)
#dl<-rnorm(52000,1800,2600)

#Define server logic
shinyServer(function(input,output){
	formulaText<-reactive({
		paste("",input$variable,sep="")
	})

	output$caption<-renderText({formulaText()})

	output$specHist<-renderPlot({
		#hist(as.formula(formulaText()),breaks=input$bins)
		with(CFrame,{
			str(CFrame)
			#hist(as.formula(formulaText()),breaks=input$bins)
			hist(input$variable,breaks=input$bins)
		})
		#hist(CFrame$Integral,breaks=input$bins)
		#hist(dl,breaks=input$bins)
	})

})
