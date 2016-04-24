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
critnames <- c("2%", "Optimal")

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
	colname <- paste(i, "ecdf", sep="-")
	P <- ecdf(timedf[[i]])
	timedf[[colname]] <- P(timedf[[i]])
}

for (i in 1:length(crit)) {
	temp <- timedf[order(timedf[[crit[i]]]), ]
	pdf(paste(filepath, "output", "plots", paste(algorithm, inst, crit[i], ".pdf", sep=""), sep="/"))
	colname <- paste(crit[i], "ecdf", sep="-")
	plot(temp[[crit[i]]], temp[[colname]],
		 main=paste("Qualified Runtime Distribution for ", algorithm, " on Instance ", inst, " [", critnames[i], "]", sep=""),
		 xlab="Runtime (s)",
		 ylab="P(solve)",
		 type="o")
	dev.off()
}









