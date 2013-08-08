library(shiny)
library(RMySQL)

#load("/export/HomeArray/home/goett/Workspace/COPPI/AutoSpec/AutoSpec.RData")
#str(CFrame)
#attach(CFrame)
#CFrame<-read.table(file='/export/HomeArray/home/goett/Workspace/COPPI/AutoSpec/xxx',header=TRUE)
#dl<-rnorm(52000,1800,2600)

driver<-dbDriver("MySQL")
dbcon<-dbConnect(MySQL(),user="goett",dbname="COPPIs")

options(shiny.maxRequestSize=-1)

#Define server logic
shinyServer(function(input,output){
	queryText<-reactive({
		x<-as.character(input$variable)
		sprintf("SELECT %s FROM autospec",x)
	})

	output$caption<-renderText({queryText()})

	queryResult<-reactive(dbGetQuery(dbcon,queryText()))

	output$specHist<-renderPlot({
		lims<-c(min(queryResult()[,1]),max(queryResult()[,1]))
		hist(queryResult()[,1],xlim=lims,breaks=input$bins)
	})

	output$downloadData <- downloadHandler(
		filename = function() {paste('xxx','.csv',sep='')},
		content = function(file) {
			write.csv(queryResult(),file)
		},
		contentType="text/csv"
	)

})
