##
##  statistics.R
##  HO-Project2
##
##  Created by Jens Nevens on 23/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- "/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2"

best <- read.table(paste(filepath, 'best.txt', sep='/'),
				   header=TRUE,
				   row.names=1)

files <- list.files(path=paste(filepath, 'output', sep='/'),
					pattern='^run')

## Compute the percentage deviation from the best know solution
## and the average percentage deviation for each algorithm
algorithm <- c()
avg <- c()

for (f in files) {
	idx <- which(strsplit(f, "")[[1]]==".")
	out.name <- substr(f,4,idx-1)
	algorithm <- c(algorithm, out.name)

	data <- read.csv(paste(filepath, 'output', f, sep="/"), 
					 header=TRUE,
					 row.names=1,
					 sep=',')
	out.data <- data.frame(row.names=rownames(best), 
						   percentage=100*((data$cost - best$cost)/best$cost))
	avg <- c(avg, mean(out.data$percentage))

	out.filename <- paste(filepath, 'percentage', paste(out.name, 'txt', sep='.'), sep="/")
	write.table(out.data, 
				file=out.filename, 
				quote=FALSE,
				col.names=FALSE)
}

avg.deviation <- data.frame(row.names=algorithm, avg=avg)
write.table(avg.deviation,
			file=paste(filepath, 'output', 'percentage-deviation.txt', sep='/'),
			quote=FALSE,
			col.names=FALSE)


## Correlation plot between the percentage deviation of the 2 algorithms
ACO <- read.table(paste(filepath, 'percentage', 'ACO.txt', sep='/'),
						header=FALSE,
						row.names=1,
						col.names=c('instance', 'percentage'))
GEN <- read.table(paste(filepath, 'percentage', 'GEN.txt', sep='/'),
						header=FALSE,
						row.names=1,
						col.names=c('instance', 'percentage'))

correlation <- round(cor(ACO$percentage, GEN$percentage), digits=3)
pdf(paste(filepath, 'output', 'plots', "correlationplot.pdf", sep='/'))
plot(ACO$percentage, GEN$percentage,
	 main=paste("Correlation between percentage deviations [", correlation, "]", sep=""),
	 xlab="ACO Percentage Deviation",
	 ylab="GEN Percentage Deviation")
abline(lm(ACO$percentage~GEN$percentage), col="red")
dev.off()

