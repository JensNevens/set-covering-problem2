##
##  correlation.R
##  HO-Project2
##
##  Created by Jens Nevens on 26/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- "/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2"

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
par(mgp=c(2.2,0.45,0), tcl=-0.4, mar=c(3.3,3.6,1.1,1.1))
plot(ACO$percentage, GEN$percentage,
	 xlab="ACO Relative Percentage Deviation",
	 ylab="GEN Relative Percentage Deviation")
abline(lm(ACO$percentage~GEN$percentage), col="red")
dev.off()

