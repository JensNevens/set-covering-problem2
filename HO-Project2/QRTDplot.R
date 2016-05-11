##
##  QRTDplot.R
##  HO-Project2
##
##  Created by Jens Nevens on 23/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- "/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2"

algorithm <- "GEN"
inst <- "D.1"
crit <- c("2", "opt")

timematrix <- matrix(nrow=25, ncol=length(crit), dimnames=list(c(), crit))

for (i in crit) {
	for (j in 0:24) {
		filename <- paste(filepath, paste("QRTD", i, sep=""), paste(paste(algorithm, j, sep=""), "txt", sep="."), sep='/')
		data <- read.csv(filename, header=TRUE, col.names=c("instance", "cost", "time"))
		timematrix[j+1,i] <- data$time[data$instance==inst]
	}
}

timedf <- as.data.frame(timematrix)

for (i in crit) {
	pdf(paste(filepath, 'output', 'plots', paste(algorithm, inst, i, '.pdf', sep=''), sep='/'))
	par(mgp=c(2.2,0.45,0), tcl=-0.4, mar=c(3.3,3.6,1.1,1.1))
	plot(ecdf(timedf[[i]]), xlab="Runtime (s)", ylab="P(solve)", main="")
	dev.off()
}










