##
##  deviation.R
##  HO-Project2
##
##  Created by Jens Nevens on 26/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- "/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2"

best <- read.table(paste(filepath, 'best.txt', sep='/'),
				   header=TRUE,
				   col.names=c('instance', 'cost'))

means <- c()
for (algorithm in c('ACO', 'GEN')) {
	cost.matrix <- matrix(nrow=62, ncol=5)
	time.matrix <- matrix(nrow=62, ncol=6)
	percentage.matrix <- matrix(nrow=62, ncol=6)

	for (i in 0:4) {
		filename <- paste(filepath, 'output', paste(algorithm, i, '.txt', sep=''), sep='/')
		data <- read.csv(filename,
						 header=TRUE,
						 col.names=c('instance', 'cost', 'time'))
		cost.matrix[,i+1] <- data$cost
		time.matrix[,i+1] <- data$time
	}

	for (i in 1:5) {
		percentage.matrix[,i] <- 100*((cost.matrix[,i] - best$cost)/best$cost)
	}

	for (j in 1:62) {
		percentage.matrix[j,6] <- mean(percentage.matrix[j,1:5])
		time.matrix[j,6] <- mean(time.matrix[j,1:5])
	}

	percentage.df <- data.frame(row.names=best$instance,
						        avg=percentage.matrix[,6])
	percentage.filename <- paste(filepath, 'percentage', paste(algorithm, 'txt', sep='.'), sep='/')
	write.table(percentage.df,
				file=percentage.filename,
				quote=FALSE,
				col.names=FALSE)
	
	means <- c(means, mean(percentage.df$avg))

	time.df <- data.frame(row.names=best$instance,
						  avg=time.matrix[,6])
	time.filename <- paste(filepath, 'time', paste(algorithm, 'txt', sep='.'), sep='/')
	write.table(time.df,
		        file=time.filename,
		        quote=FALSE,
		        col.names=FALSE)
}

means.df <- data.frame(row.names=c('ACO', 'GEN'),
					   avg=means)
write.table(means.df,
			file=paste(filepath, 'output', 'percentage-deviation.txt', sep='/'),
			quote=FALSE,
			col.names=FALSE)
