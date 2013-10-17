library(shiny)
library(RMySQL)

driver<-dbDriver("MySQL")
dbcon<-dbConnect(MySQL(),user="goett",dbname="COPPIs")

options(shiny.maxRequestSize=-1)

#Define server logic
shinyServer(function(input,output){

	queryText<-reactive({
		x<-as.character(input$variable)
		y<-paste(input$Channels,sep="",collapse=" OR ")
		sprintf("SELECT * FROM autospec WHERE %s",x,y)
	})

	plotText<-reactive({
		x<-as.character(input$variable)
		sprintf('queryResult$%s',x)
	})

	output$caption<-renderText({queryText()})

	queryResult<-reactive(dbGetQuery(dbcon,queryText()))

	output$specHist<-renderPlot({
			lims<-c(min(queryResult()[,1]),max(queryResult()[,1]))
			x<-paste("queryResult()","$",input$variable,sep="")
			hist(x,xlim=lims,breaks=input$bins)
	})

	output$summary<-renderPrint({
		summary(queryResult())
	})

	output$downloadData <- downloadHandler(
		filename = function() {paste('xxx','.csv',sep='')},
		content = function(file) {
			write.csv(queryResult(),file)
		},
		contentType="text/csv"
	)

})
